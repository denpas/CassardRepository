#ifndef CASSARD_I2C_CONTROLER_H
#define CASSARD_I2C_CONTROLER_H

#include <Arduino.h>
#include <Wire.h>
#include <util/atomic.h>
#include "Protocol.h"

const byte I2CCommandMaxLength = 8;
const byte I2CRxBufferLength = 64;

volatile byte i2cRxBuffer[I2CRxBufferLength];
volatile byte i2cRxHead = 0;
volatile byte i2cRxTail = 0;
volatile bool i2cRxOverflow = false;

class I2CControler
{
public:
    I2CControler();
    void SetId(String id);
    void SetModuleNum(int moduleNum);
    void Update();
    bool NewCommandAvailable(byte command[I2CCommandMaxLength], byte& commandLength);

private:
    String _id;
    byte _currentCommand[I2CCommandMaxLength];
    byte _lastWritten;
    bool _discardUntilEnd;
};

void ReceiveEvent(int howMany)
{
    (void)howMany;
    while (Wire.available() > 0)
    {
        byte value = Wire.read();
        byte nextHead = (i2cRxHead + 1) % I2CRxBufferLength;
        if (nextHead == i2cRxTail)
        {
            i2cRxOverflow = true;
        }
        else
        {
            i2cRxBuffer[i2cRxHead] = value;
            i2cRxHead = nextHead;
        }
    }
}

I2CControler::I2CControler()
{
    _id = "I2CControler";
    _lastWritten = 0;
    _discardUntilEnd = false;
    Wire.onReceive(ReceiveEvent);
}

void I2CControler::SetId(String id)
{
    _id = id;
}

void I2CControler::SetModuleNum(int moduleNum)
{
    Wire.begin(moduleNum);
}

bool I2CControler::NewCommandAvailable(byte command[I2CCommandMaxLength], byte& commandLength)
{
    commandLength = 0;
    bool overflowed = false;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        overflowed = i2cRxOverflow;
        i2cRxOverflow = false;
    }
    if (overflowed)
    {
        _lastWritten = 0;
        _discardUntilEnd = true;
    }

    while (i2cRxTail != i2cRxHead)
    {
        byte value = i2cRxBuffer[i2cRxTail];
        i2cRxTail = (i2cRxTail + 1) % I2CRxBufferLength;

        if (_discardUntilEnd)
        {
            if (value == END_OF_TRAME)
                _discardUntilEnd = false;
            continue;
        }

        if (value == END_OF_TRAME)
        {
            for (byte index = 0; index < _lastWritten; index++)
                command[index] = _currentCommand[index];
            command[_lastWritten] = value;
            commandLength = _lastWritten + 1;
            _lastWritten = 0;
            return true;
        }

        if (_lastWritten >= I2CCommandMaxLength - 1)
        {
            _lastWritten = 0;
            _discardUntilEnd = true;
            continue;
        }

        _currentCommand[_lastWritten++] = value;
    }

    return false;
}

void I2CControler::Update()
{
}

#endif
