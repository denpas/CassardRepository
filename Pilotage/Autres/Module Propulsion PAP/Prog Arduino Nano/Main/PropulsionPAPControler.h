//#ifndef PropulsionPAPControler_h
//#define PropulsionPAPControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LedControler.h"
#include "StepMotorControler.h"
#include "ServoControler.h"

struct PropulsionPAPParams
{
	String id;
	int okLedPin;
	int errorLedPin;
	int controlerLeftEngineClockPin;	
	int controlerLeftEngineEnablePin;	
	int controlerLeftEngineDirectionPin;
	int controlerRightEngineClockPin;
	int controlerRightEngineEnablePin;
	int controlerRightEngineDirectionPin;
	int servoDirectionPin;
	int delayLeftEngine;
	int delayRightEngine;
	int directionSpeed;
};
PropulsionPAPParams propulsionPAPParams;
bool unChangement;
int PropulsionPAPLeftEngineResult;
int PropulsionPAPRightEngineResult;
int PropulsionDirectionResult;

class PropulsionPAPControler
{
    public:
        PropulsionPAPControler();
		void Setup(PropulsionPAPParams propulsionPAPParams);
		void SetEngineSpeed(int engineSpeed);
		void StopEngine();
		void SetDirection(int direction);
		void ForwardEngine();
		void ReverseEngine();
        void Update();
		void SetId(String id);
		void SetChangeSpeed(int delayLeftEngine, int delayRightEngine, int delayDirection);
		void ErrorValueOutOfRange();
		void Ok();
	private:
		String _id;
		int _delayLeftEngine;
		int _delayRightEngine;
		int _delayDirection;

		StepMotorControler PropulsionPAPLeftEngine;		// Création de l'objet StepMotor pour la PropulsionPAP du moteur Gauche
		StepMotorControler PropulsionPAPRightEngine;	// Création de l'objet StepMotor pour la PropulsionPAP du moteur Droit
		ServoControler PropulsionDirection;				// Création de l'objet servo pour la Direction
		LedControler LedCommandeOk;						// Création de l'objet led verte pour commande acceptée
		LedControler LedCommandeError;					// Création de l'objet led verte pour commande refusée
};

//---------------------------------------------------------------------------------------------------------------------------------------
PropulsionPAPControler::PropulsionPAPControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_delayLeftEngine = 0;
	_delayRightEngine = 0;
	_delayDirection = 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::SetChangeSpeed(int delayLeftEngine, int delayRightEngine, int delayDirection)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_delayLeftEngine = delayLeftEngine;
	_delayRightEngine = delayRightEngine;
	_delayDirection = delayDirection;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::ErrorValueOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	LedCommandeOk.SetState(LED_OFF);
	LedCommandeError.SetDelayOn(50);
	LedCommandeError.SetDelayOff(100);
	LedCommandeError.BlinkNTimes(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::Ok()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	LedCommandeOk.SetState(LED_ON);
	LedCommandeError.SetDelayOn(500);
	LedCommandeError.SetDelayOff(500);
	LedCommandeError.SetState(LED_OFF);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::Setup(PropulsionPAPParams PropulsionPAPParams)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SetId(PropulsionPAPParams.id);

	LedCommandeOk.SetId("Led Ok");
	LedCommandeOk.SetDefault(false);
	SerialLog.LogTV("["+ PropulsionPAPParams.id+ "] Attached okLed to pin :", PropulsionPAPParams.okLedPin);
	LedCommandeOk.AttachToPin(PropulsionPAPParams.okLedPin);					

	LedCommandeError.SetId("Led Error");
	LedCommandeError.SetDefault(false);
	SerialLog.LogTV("["+ PropulsionPAPParams.id + "] Attached errorLed to pin :", PropulsionPAPParams.errorLedPin);
	LedCommandeError.AttachToPin(PropulsionPAPParams.errorLedPin);			

	PropulsionPAPLeftEngine.SetId("Controler left engine");
	PropulsionPAPLeftEngine.SetDefault();
	if (PropulsionPAPLeftEngine.SetIntervalBetweenUpdates(PropulsionPAPParams.delayLeftEngine) == 0)
	{
		SerialLog.LogTV("["+ PropulsionPAPParams.id + "] Attached controlerLeftEngine to pin :", PropulsionPAPParams.controlerLeftEngineClockPin);
		PropulsionPAPLeftEngine.AttachToPin(PropulsionPAPParams.controlerLeftEngineClockPin,
											PropulsionPAPParams.controlerLeftEngineEnablePin, 
											PropulsionPAPParams.controlerLeftEngineDirectionPin);

		PropulsionPAPRightEngine.SetId("Controler right engine");
		PropulsionPAPRightEngine.SetDefault();
		if (PropulsionPAPRightEngine.SetIntervalBetweenUpdates(PropulsionPAPParams.delayRightEngine) == 0)
		{
			SerialLog.LogTV("["+ PropulsionPAPParams.id + "] Attached controlerRightEngine to pin :", PropulsionPAPParams.controlerRightEngineClockPin);
			PropulsionPAPRightEngine.AttachToPin(	PropulsionPAPParams.controlerRightEngineClockPin,
													PropulsionPAPParams.controlerRightEngineEnablePin,
													PropulsionPAPParams.controlerRightEngineDirectionPin);

			PropulsionDirection.SetId("Controler direction");
			PropulsionDirection.SetDefault(false);
			if (PropulsionDirection.SetSpeed(PropulsionPAPParams.directionSpeed) == 0)
			{
				if (PropulsionDirection.SetRange(0, 180) == 0)
				{
					SerialLog.LogTV("[" + PropulsionPAPParams.id + "] Attached servoDirection to pin :", PropulsionPAPParams.servoDirectionPin);
					PropulsionDirection.AttachToPin(PropulsionPAPParams.servoDirectionPin);

					Ok();
				}
			}
			else ErrorValueOutOfRange();
		}
		else ErrorValueOutOfRange();
	} 
	else  ErrorValueOutOfRange();

}


//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::SetEngineSpeed(int engineSpeed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionPAPRightEngine.SetIntervalBetweenUpdates(engineSpeed) == 0)
	{
		if (PropulsionPAPLeftEngine.SetIntervalBetweenUpdates(engineSpeed) == 0)
		{
			Ok();
		}
		else ErrorValueOutOfRange();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::StopEngine()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionPAPRightEngine.Stop() == 0)
	{
		if (PropulsionPAPLeftEngine.Stop() == 0)
		{
			Ok();
		}
		else ErrorValueOutOfRange();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::ForwardEngine()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionPAPRightEngine.TurnLeftStep(0) == 0)
	{
		if (PropulsionPAPLeftEngine.TurnRightStep(0) == 0)
		{
			Ok();
		}
		else ErrorValueOutOfRange();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::ReverseEngine()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionPAPRightEngine.TurnRightStep(0) == 0)
	{
		if (PropulsionPAPLeftEngine.TurnLeftStep(0) == 0)
		{
			Ok();
		}
		else ErrorValueOutOfRange();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::SetDirection(int direction)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionDirection.SetPosition(direction,false) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionPAPControler::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	unChangement = false;
	PropulsionPAPLeftEngineResult = PropulsionPAPLeftEngine.Update();
	PropulsionPAPRightEngineResult = PropulsionPAPRightEngine.Update();
	if ((PropulsionPAPLeftEngineResult == 1) || (PropulsionPAPLeftEngineResult == -1)) unChangement = true;
	if ((PropulsionPAPRightEngineResult == 1) || (PropulsionPAPRightEngineResult == -1)) unChangement = true;
	if ((PropulsionDirectionResult == 1) || (PropulsionDirectionResult == -1)) unChangement = true;

	if (unChangement)
	{
		LedCommandeOk.SetState(LED_ON); 
	} else
	{
		if ((PropulsionPAPLeftEngineResult != -2) && (PropulsionPAPRightEngineResult != -2) && (PropulsionDirectionResult != -2))
		{ 
			LedCommandeOk.SetState(LED_OFF);
		}
	}
	LedCommandeOk.Update();
	LedCommandeError.Update();
}

//#endif
