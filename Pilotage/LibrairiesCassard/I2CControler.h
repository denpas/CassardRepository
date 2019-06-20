#ifndef I2CControler_h
#define I2CControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "Wire.h"
#include "RingBuf.h"

const int I2CCommandMaxLenght = 20;

// Declare as volatile, since modofied in ISR
volatile unsigned int index = 0;

// Create a RinBuf object designed to hold a 20 Event structs
RingBuf *buf = RingBuf_new(1,500);

class I2CControler
{
    public:
        I2CControler();
        void SetId(String id);
		void SetModuleNum(int moduleNum);
        void Update();
		bool NewCommandAvailable(unsigned char command[I2CCommandMaxLenght]);
    private:
        String _id;
		int _moduleNum;
		int _lastWritten;
		byte _currentCommand[I2CCommandMaxLenght];
};


//---------------------------------------------------------------------------------------------------------------------------------------
void ReceiveEvent(int howMany) 
//---------------------------------------------------------------------------------------------------------------------------------------
{
	byte value;
	while (Wire.available() > 0) 
	{
		 // Add it to the buffer
		value = Wire.read();
		buf->add(buf, &value);
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------
I2CControler::I2CControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_id = "I2CControler";
	this->_lastWritten = 0;
	// Check if null pointer
	if (!buf)
	{
		SerialLog.Log("Not enough memory");
		while (1);
	}
	Wire.onReceive(ReceiveEvent); // register event
}

//---------------------------------------------------------------------------------------------------------------------------------------
void I2CControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void I2CControler::SetModuleNum(int moduleNum)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_moduleNum = moduleNum;
	Wire.begin(_moduleNum);               // join i2c bus with address #id
}
//---------------------------------------------------------------------------------------------------------------------------------------
bool I2CControler::NewCommandAvailable(unsigned char command[I2CCommandMaxLenght])
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int index = 0;
	bool endOfCommand = false;
	byte value;
	
	while ((this->_lastWritten < I2CCommandMaxLenght) && (buf->pull(buf, &value)) && (!endOfCommand))
	{
		this->_currentCommand[this->_lastWritten] = value;
		endOfCommand = (value == 255);
		if (!endOfCommand)
		{
			this->_lastWritten++;
		}
	}
	if (endOfCommand)
	{
		for (index = 0; (index <= this->_lastWritten); index++) command[index] = this->_currentCommand[index];
		this->_lastWritten = 0;
		return true;
	} else return false;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void I2CControler::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
}

#endif
