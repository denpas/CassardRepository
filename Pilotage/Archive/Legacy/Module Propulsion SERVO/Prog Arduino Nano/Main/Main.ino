/*
Programme de test Propulsion 
Pascal DENICOU
*/

//#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LogControler.h"
#include "PropulsionBRUControler.h"
#include "I2CControler.h"


//***************************************************************************************************************************************
// M A I N 
//***************************************************************************************************************************************

const int NumeroModuleI2C = 2;
const int IndexModule = 0;
const int IndexCommand = 1;
PropulsionBRUControler Propulsion;          // Création de l'objet Propulsion 
PropulsionBRUParams PropulsionBRUParams;
I2CControler I2C;
char newCommand[I2CCommandLenght];

//***************************************************************************************************************************************
void setup() {
//***************************************************************************************************************************************
	PropulsionBRUParams.id = "Propulsion";

	PropulsionBRUParams.okLedPin = 6;
	PropulsionBRUParams.errorLedPin = 5;

	PropulsionBRUParams.leftEngineControlerPin = 13;
	PropulsionBRUParams.rightEngineControlerPin = 14;
	PropulsionBRUParams.directionControlerPin = 10;

	PropulsionBRUParams.leftEngineSpeed = 254;		// 254 à remplacer par une valeur donnant une vitesse de rotation réaliste
	PropulsionBRUParams.rightEngineSpeed = 254;	// 254 à remplacer par une valeur donnant une vitesse de rotation réaliste
	PropulsionBRUParams.directionSpeed = 254;	// 254 à remplacer par une valeur donnant une vitesse de rotation réaliste

	Propulsion.Setup(PropulsionBRUParams);
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
	if (newCommand[IndexModule] == NumeroModuleI2C)
	{
		//ForwardEngine
		if (newCommand[IndexCommand] == 12)
		{
			Propulsion.ForwardEngine(newCommand[2]);
			I2C.EmptyCommand();
		}
		//ReverseEngine
		if (newCommand[IndexCommand] == 13)
		{
			Propulsion.ReverseEngine(newCommand[2]);
			I2C.EmptyCommand();
		}
		//StopEngine
		if (newCommand[IndexCommand] == 14)
		{
			Propulsion.StopEngine();
			I2C.EmptyCommand();
		}
		//TurnRight
		if (newCommand[IndexCommand] == 15)
		{
			Propulsion.TurnRight(newCommand[2]);
			I2C.EmptyCommand();
		}
		//TurnLeft
		if (newCommand[IndexCommand] == 16)
		{
			Propulsion.TurnLeft(newCommand[2]);
			I2C.EmptyCommand();
		}
		//GoStraight
		if (newCommand[IndexCommand] == 17)
		{
			Propulsion.GoStraight();
			I2C.EmptyCommand();
		}
	}
	else ErrorModuleOutOfRange();

}

//***************************************************************************************************************************************
void loop()
{
	Propulsion.Update();
	I2C.Update();
	if (I2C.NewCommandAvailable(newCommand)) ProcessCommand();
}
//---------------------------------------------------------------------------------------------------------------------------------------

