/*
Programme de test Propulsion 
Pascal DENICOU
*/
/*
//#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LogControler.h"
#include "PropulsionPAPControler.h"
#include "I2CControler.h"


//***************************************************************************************************************************************
// M A I N 
//***************************************************************************************************************************************

const int NumeroModuleI2C = 2;
const int IndexModule = 0;
const int IndexCommand = 1;
PropulsionPAPControler Propulsion;          // Création de l'objet Propulsion 
PropulsionPAPParams PropulsionPAPParams;
I2CControler I2C;
unsigned char newCommand[I2CCommandLenght];

//***************************************************************************************************************************************
void setup() {
//***************************************************************************************************************************************
	PropulsionPAPParams.id = "Propulsion";
	PropulsionPAPParams.okLedPin = 5;
	PropulsionPAPParams.errorLedPin = 6;
	PropulsionPAPParams.controlerLeftEngineClockPin = 8;
	PropulsionPAPParams.controlerLeftEngineEnablePin = 7;
	PropulsionPAPParams.controlerLeftEngineDirectionPin = 9;
	PropulsionPAPParams.controlerRightEngineClockPin = 2;
	PropulsionPAPParams.controlerRightEngineEnablePin = 4;
	PropulsionPAPParams.controlerRightEngineDirectionPin = 3;
	PropulsionPAPParams.servoDirectionPin = 14;
	PropulsionPAPParams.delayLeftEngine = 1;	// 0 à remplacer par une valeur donnant une vitesse de rotation réaliste
	PropulsionPAPParams.delayRightEngine = 1;	// 0 à remplacer par une valeur donnant une vitesse de rotation réaliste
	Propulsion.Setup(PropulsionPAPParams);
	Propulsion.SetEngineSpeed(0);
	Propulsion.SetDirection(90);
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
		if (newCommand[IndexCommand] == 11)
		{
			Propulsion.SetEngineSpeed(newCommand[2]);
			I2C.EmptyCommand();
		}
		if (newCommand[IndexCommand] == 12)
		{
			Propulsion.StopEngine();
			I2C.EmptyCommand();
		}
		if (newCommand[IndexCommand] == 13)
		{
			Propulsion.SetDirection(newCommand[2]);
			I2C.EmptyCommand();
		}
		if (newCommand[IndexCommand] == 15)
		{
			Propulsion.ReverseEngine();
			I2C.EmptyCommand();
		}
		if (newCommand[IndexCommand] == 14)
		{
			Propulsion.ForwardEngine();
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
*/
#include "AccelStepper.h"

// Define some steppers and the pins the will use
AccelStepper stepper1(1,2,3); 
AccelStepper stepper2(1,8,9);

void setup()
{
	stepper1.setMaxSpeed(4000.0);
	stepper1.setSpeed(4000.0);
	stepper1.setAcceleration(4000.0);
	stepper1.moveTo(4800);

	stepper2.setMaxSpeed(4000.0);
	stepper2.setSpeed(4000.0);
	stepper2.setAcceleration(4000.0);
	stepper2.moveTo(4800);
}

void loop()
{
	// Change direction at the limits
	if (stepper1.distanceToGo() == 0)
		stepper1.moveTo(-stepper1.currentPosition());
	stepper1.run();

	if (stepper2.distanceToGo() == 0)
		stepper2.moveTo(-stepper2.currentPosition());
	stepper2.run();
}
