#include <CassardControl.h>
#include <CassardControl/LogControler.h>
#include "TourelleControler.h"
#include <CassardControl/I2CControler.h>
#include <CassardControl/Protocol.h>

const byte IndexModule = 0;
const byte IndexCommand = 1;
const bool simulation = false;

TourelleControler TourelleCanon100mm;
TourelleParams TourelleCanon100mmParams;
I2CControler I2C;
byte newCommand[I2CCommandMaxLength];

void setup()
{
    I2C.SetId("TOURELLE I2CControler");
    I2C.SetModuleNum(MODULE_TOURELLE);

    TourelleCanon100mmParams.id = "TourelleCanon100mm";
    TourelleCanon100mmParams.okLedPin = 6;
    TourelleCanon100mmParams.errorLedPin = 5;
    TourelleCanon100mmParams.servoAzimutPin = 14;
    TourelleCanon100mmParams.servoElevationPin = 13;
    TourelleCanon100mmParams.smokePin = 0;
    TourelleCanon100mmParams.soundPin = 0;
    TourelleCanon100mmParams.azimutSpeed = MaxSpeed;
    TourelleCanon100mmParams.elevationSpeed = MaxSpeed;
    TourelleCanon100mm.Setup(TourelleCanon100mmParams);
    TourelleCanon100mm.GoToPos(0, 0);
}

void ProcessCommand(byte commandLength)
{
    if (commandLength != 5 ||
        newCommand[commandLength - 1] != END_OF_TRAME ||
        newCommand[IndexModule] != MODULE_TOURELLE ||
        newCommand[IndexCommand] != TOURELLE_SETAZIMUTELEVATION)
    {
        TourelleCanon100mm.ErrorValueOutOfRange();
        return;
    }

    TourelleCanon100mm.GoToPos(newCommand[2], newCommand[3]);
}

void loop()
{
    if (simulation)
    {
        newCommand[0] = MODULE_TOURELLE;
        newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
        newCommand[2] = 30;
        newCommand[3] = 30;
        newCommand[4] = END_OF_TRAME;
        ProcessCommand(5);
        delay(1000);
        TourelleCanon100mm.Update();
        delay(3000);

        newCommand[0] = MODULE_TOURELLE;
        newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
        newCommand[2] = 30;
        newCommand[3] = 0;
        newCommand[4] = END_OF_TRAME;
        ProcessCommand(5);
        delay(1000);
        TourelleCanon100mm.Update();
        delay(3000);

        newCommand[0] = MODULE_TOURELLE;
        newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
        newCommand[2] = 0;
        newCommand[3] = 30;
        newCommand[4] = END_OF_TRAME;
        ProcessCommand(5);
        delay(1000);
        TourelleCanon100mm.Update();
        delay(3000);

        newCommand[0] = MODULE_TOURELLE;
        newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
        newCommand[2] = 0;
        newCommand[3] = 0;
        newCommand[4] = END_OF_TRAME;
        ProcessCommand(5);
        delay(1000);
        TourelleCanon100mm.Update();
        delay(3000);
    }
    else
    {
        I2C.Update();
        byte commandLength;
        if (I2C.NewCommandAvailable(newCommand, commandLength))
            ProcessCommand(commandLength);
        TourelleCanon100mm.Update();
    }
}
