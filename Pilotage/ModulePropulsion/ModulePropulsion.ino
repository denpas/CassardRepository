/*
Programme de test Propulsion 
Pascal DENICOU
*/

//#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LogControler.h"
#include "PropulsionCONTControler.h"
#include "I2CControler.h"
#include "Protocol.h"


//***************************************************************************************************************************************
// M A I N 
//***************************************************************************************************************************************

const int IndexModule = 0;
const int IndexCommand = 1;
bool simulation = false;

PropulsionCONTControler Propulsion;          // Création de l'objet Propulsion 
PropulsionCONTParams PropulsionCONTParams;
I2CControler I2C;
byte newCommand[I2CCommandMaxLenght];

//***************************************************************************************************************************************
void setup() {
//***************************************************************************************************************************************
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
	if (newCommand[IndexModule] == MODULE_PROPULSION)
	{
		//ForwardEngine
		if (newCommand[IndexCommand] == PROPULSION_FORWARDENGINE)
		{
			Propulsion.ForwardEngine(newCommand[2]);
		}
		//ReverseEngine
		if (newCommand[IndexCommand] == PROPULSION_REVERSEENGINE)
		{
			Propulsion.ReverseEngine(newCommand[2]);
		}
		//StopEngine
		if (newCommand[IndexCommand] == PROPULSION_STOPENGINE)
		{
			Propulsion.StopEngine();
		}
		//TurnRight
		if (newCommand[IndexCommand] == PROPULSION_TURNRIGHT)
		{
			Propulsion.TurnRight(newCommand[2]);
		}
		//TurnLeft
		if (newCommand[IndexCommand] == PROPULSION_TURNLEFT)
		{
			Propulsion.TurnLeft(newCommand[2]);
		}
		//GoStraight
		if (newCommand[IndexCommand] == PROPULSION_GOSTRAIGHT)
		{
			Propulsion.GoStraight();
		}
		//SetPropulsionMode
		if (newCommand[IndexCommand] == PROPULSION_SETPROPORTIONALMODE)
		{
			Propulsion.SetProportionalMode(newCommand[2]);
		}
	}
	else ErrorModuleOutOfRange();
}

//***************************************************************************************************************************************
void loop()
{
	simulation = false;
	
	if (simulation)
	{	
		newCommand[0] = MODULE_PROPULSION;
		newCommand[1] = PROPULSION_FORWARDENGINE;
		newCommand[2] = 80;
		newCommand[3] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		Propulsion.Update();
		delay(3000);

		newCommand[0] = MODULE_PROPULSION;
		newCommand[1] = PROPULSION_TURNRIGHT;
		newCommand[2] = 60;
		newCommand[3] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		Propulsion.Update();
		delay(3000);

		newCommand[0] = MODULE_PROPULSION;
		newCommand[1] = PROPULSION_GOSTRAIGHT;
		newCommand[2] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		Propulsion.Update();
		delay(3000);

		newCommand[0] = MODULE_PROPULSION;
		newCommand[1] = PROPULSION_TURNLEFT;
		newCommand[2] = 60;
		newCommand[3] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		Propulsion.Update();
		delay(3000);

		newCommand[0] = MODULE_PROPULSION;
		newCommand[1] = PROPULSION_STOPENGINE;
		newCommand[2] = END_OF_TRAME;
		ProcessCommand();
		delay(1000);
		Propulsion.Update();
		delay(3000);
	} else
	{
		Propulsion.Update();
		I2C.Update();
		if (I2C.NewCommandAvailable(newCommand)) ProcessCommand();
	}	
}
//---------------------------------------------------------------------------------------------------------------------------------------
