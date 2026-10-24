#include <CassardControl/LogControler.h>
#include "PropulsionCONTControler.h"
#include <CassardControl/I2CControler.h>
#include <CassardControl/Protocol.h>

const byte IndexModule = 0;
const byte IndexCommand = 1;
const bool simulation = false;

PropulsionCONTControler Propulsion;
PropulsionCONTParams PropulsionCONTParams;
I2CControler I2C;
byte newCommand[I2CCommandMaxLength];

void setup()
{
    I2C.SetId("PROPULSION I2CControler");
    I2C.SetModuleNum(MODULE_PROPULSION);

    PropulsionCONTParams.id = "Propulsion";
    PropulsionCONTParams.okLedPin = 16;
    PropulsionCONTParams.errorLedPin = 15;
    PropulsionCONTParams.leftEngineDirectionPin = 9;
    PropulsionCONTParams.leftEngineSpeedPin = 3;
    PropulsionCONTParams.rightEngineDirectionPin = 12;
    PropulsionCONTParams.rightEngineSpeedPin = 5;
    PropulsionCONTParams.directionPin = 14;
    PropulsionCONTParams.proportionalMode = 0;
    Propulsion.Setup(PropulsionCONTParams);
}

void ProcessCommand(byte commandLength)
{
    if (commandLength < 3 ||
        newCommand[commandLength - 1] != END_OF_TRAME ||
        newCommand[IndexModule] != MODULE_PROPULSION)
    {
        Propulsion.ErrorValueOutOfRange();
        return;
    }

    switch (newCommand[IndexCommand])
    {
    case PROPULSION_FORWARDENGINE:
        if (commandLength == 4)
            Propulsion.ForwardEngine(newCommand[2]);
        else
            Propulsion.ErrorValueOutOfRange();
        break;
    case PROPULSION_REVERSEENGINE:
        if (commandLength == 4)
            Propulsion.ReverseEngine(newCommand[2]);
        else
            Propulsion.ErrorValueOutOfRange();
        break;
    case PROPULSION_STOPENGINE:
        if (commandLength == 3)
            Propulsion.StopEngine();
        else
            Propulsion.ErrorValueOutOfRange();
        break;
    case PROPULSION_TURNRIGHT:
        if (commandLength == 4)
            Propulsion.TurnRight(newCommand[2]);
        else
            Propulsion.ErrorValueOutOfRange();
        break;
    case PROPULSION_TURNLEFT:
        if (commandLength == 4)
            Propulsion.TurnLeft(newCommand[2]);
        else
            Propulsion.ErrorValueOutOfRange();
        break;
    case PROPULSION_GOSTRAIGHT:
        if (commandLength == 3)
            Propulsion.GoStraight();
        else
            Propulsion.ErrorValueOutOfRange();
        break;
    case PROPULSION_SETPROPORTIONALMODE:
        if (commandLength == 4)
            Propulsion.SetProportionalMode(newCommand[2]);
        else
            Propulsion.ErrorValueOutOfRange();
        break;
    default:
        Propulsion.ErrorValueOutOfRange();
        break;
    }
}

void loop()
{
    if (simulation)
    {
        newCommand[0] = MODULE_PROPULSION;
        newCommand[1] = PROPULSION_FORWARDENGINE;
        newCommand[2] = 80;
        newCommand[3] = END_OF_TRAME;
        ProcessCommand(4);
        delay(1000);
        Propulsion.Update();
        delay(3000);

        newCommand[0] = MODULE_PROPULSION;
        newCommand[1] = PROPULSION_TURNRIGHT;
        newCommand[2] = 60;
        newCommand[3] = END_OF_TRAME;
        ProcessCommand(4);
        delay(1000);
        Propulsion.Update();
        delay(3000);

        newCommand[0] = MODULE_PROPULSION;
        newCommand[1] = PROPULSION_GOSTRAIGHT;
        newCommand[2] = END_OF_TRAME;
        ProcessCommand(3);
        delay(1000);
        Propulsion.Update();
        delay(3000);

        newCommand[0] = MODULE_PROPULSION;
        newCommand[1] = PROPULSION_TURNLEFT;
        newCommand[2] = 60;
        newCommand[3] = END_OF_TRAME;
        ProcessCommand(4);
        delay(1000);
        Propulsion.Update();
        delay(3000);

        newCommand[0] = MODULE_PROPULSION;
        newCommand[1] = PROPULSION_STOPENGINE;
        newCommand[2] = END_OF_TRAME;
        ProcessCommand(3);
        delay(1000);
        Propulsion.Update();
        delay(3000);
    }
    else
    {
        Propulsion.Update();
        I2C.Update();
        byte commandLength;
        if (I2C.NewCommandAvailable(newCommand, commandLength))
            ProcessCommand(commandLength);
    }
}
