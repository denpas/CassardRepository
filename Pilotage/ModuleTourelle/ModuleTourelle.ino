/*
Programme de test tourelle canon 100mm
Pascal DENICOU
Sources sur GIT
*/

//#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LogControler.h"
#include "TourelleControler.h"
#include "I2CControler.h"
#include "Protocol.h"


//***************************************************************************************************************************************
// M A I N 
//***************************************************************************************************************************************

const int IndexModule = 0;
const int IndexCommand = 1;
bool simulation = false;
TourelleControler TourelleCanon100mm;          // Création de l'objet tourelle du canon de 100mm
TourelleParams TourelleCanon100mmParams;
I2CControler I2C;
byte newCommand[I2CCommandMaxLenght];

//***************************************************************************************************************************************
void setup() {
//***************************************************************************************************************************************
	I2C.SetId("TOURELLE I2CControler");
	I2C.SetModuleNum(MODULE_TOURELLE);

	TourelleCanon100mmParams.id = "TourelleCanon100mm";
	TourelleCanon100mmParams.okLedPin = 6;
	TourelleCanon100mmParams.errorLedPin = 5;
	TourelleCanon100mmParams.servoAzimutPin = 14;
	TourelleCanon100mmParams.servoElevationPin = 13;
	TourelleCanon100mmParams.smokePin = 0;
	TourelleCanon100mmParams.soundPin = 0;
	TourelleCanon100mmParams.azimutSpeed = MaxSpeed;	// remplacer par une valeur donnant une vitesse de rotation réaliste
	TourelleCanon100mmParams.elevationSpeed = MaxSpeed;	// 0 à remplacer par une valeur donnant une vitesse de rotation réaliste
	TourelleCanon100mm.Setup(TourelleCanon100mmParams);
	TourelleCanon100mm.GoToPos(0, 0);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ErrorModuleOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{

}

//---------------------------------------------------------------------------------------------------------------------------------------
void ProcessCommand()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	// Vérification du numéro de module
	if (newCommand[IndexModule] == MODULE_TOURELLE)
	{
		if (newCommand[IndexCommand] == TOURELLE_SETUP)
		{
			//Setup
			
		}
		if (newCommand[IndexCommand] == TOURELLE_SETAZIMUTELEVATION)
		{
			//SetAzimutElevation
			TourelleCanon100mm.GoToPos(newCommand[2], newCommand[3]);
		}
	}
	else ErrorModuleOutOfRange();
}

//***************************************************************************************************************************************
void loop()
{
	// A modifier si besoin !
	simulation = false;
	
	if (simulation)
	{	
		newCommand[0] = MODULE_TOURELLE;
		newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
		newCommand[2] = 30;
		newCommand[3] = 30;
		newCommand[4] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		TourelleCanon100mm.Update();
		delay(3000);

		newCommand[0] = MODULE_TOURELLE;
		newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
		newCommand[2] = 30;
		newCommand[3] = 0;
		newCommand[4] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		TourelleCanon100mm.Update();
		delay(3000);

		newCommand[0] = MODULE_TOURELLE;
		newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
		newCommand[2] = -30;
		newCommand[3] = 30;
		newCommand[4] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		TourelleCanon100mm.Update();
		delay(3000);

		newCommand[0] = MODULE_TOURELLE;
		newCommand[1] = TOURELLE_SETAZIMUTELEVATION;
		newCommand[2] = -30;
		newCommand[3] = 0;
		newCommand[4] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		TourelleCanon100mm.Update();
		delay(3000);
	} else
	{
		I2C.Update();
		if (I2C.NewCommandAvailable(newCommand)) ProcessCommand();	
	}	
}
//---------------------------------------------------------------------------------------------------------------------------------------

