//#ifndef PropulsionController_h
//#define PropulsionController_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LedController.h"
#include "ServoController.h"

struct PropulsionParams
{
	String id;
	int okLedPin;
	int errorLedPin;
	int leftEngineControlerPin;
	int rightEngineControlerPin;
	int directionControlerPin;
	int delayLeftEngine;
	int delayRightEngine;
	int delayDirection;
};
PropulsionParams propulsionParams;
bool unChangement;
int PropulsionLeftEngineResult;
int PropulsionRightEngineResult;
int PropulsionDirectionResult;

class PropulsionController
{
    public:
        PropulsionController();
		void Setup(PropulsionParams propulsionParams);
		void SetEngineSpeed(int rightEngineSpeed, int leftEngineSpeed);
		void SetDirection(int direction);
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

		ServoController PropulsionLeftEngine; // Création de l'objet servo pour la Propulsion du moteur Gauche
		ServoController PropulsionRightEngine;// Création de l'objet servo pour la Propulsion du moteur Droit
		ServoController PropulsionDirection;  // Création de l'objet servo pour la Direction
		LedController LedCommandeOk;          // Création de l'objet led verte pour commande acceptée
		LedController LedCommandeError;       // Création de l'objet led verte pour commande refusée
};

//---------------------------------------------------------------------------------------------------------------------------------------
PropulsionController::PropulsionController()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_delayLeftEngine = 0;
	_delayRightEngine = 0;
	_delayDirection = 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::SetChangeSpeed(int delayLeftEngine, int delayRightEngine, int delayDirection)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_delayLeftEngine = delayLeftEngine;
	_delayRightEngine = delayRightEngine;
	_delayDirection = delayDirection;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::ErrorValueOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	LedCommandeOk.SetState(LED_OFF);
	LedCommandeError.SetDelayOn(50);
	LedCommandeError.SetDelayOff(100);
	LedCommandeError.BlinkNTimes(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::Ok()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	LedCommandeOk.SetState(LED_ON);
	LedCommandeError.SetDelayOn(500);
	LedCommandeError.SetDelayOff(500);
	LedCommandeError.SetState(LED_OFF);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::Setup(PropulsionParams propulsionParams)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SetId(propulsionParams.id);

	LedCommandeOk.SetId("Led Ok");
	LedCommandeOk.SetDefault(false);
	SerialLog.LogTV("["+ propulsionParams.id+ "] Attached okLed to pin :", propulsionParams.okLedPin);
	LedCommandeOk.AttachToPin(propulsionParams.okLedPin);					// attache la pin 2 à l'objet led LedCommandeOk

	LedCommandeError.SetId("Led Error");
	LedCommandeError.SetDefault(false);
	SerialLog.LogTV("["+ propulsionParams.id + "] Attached errorLed to pin :", propulsionParams.errorLedPin);
	LedCommandeError.AttachToPin(propulsionParams.errorLedPin);			// attache la pin 13 à l'objet led LedCommandeError

	PropulsionLeftEngine.SetId("Controler left engine");
	PropulsionLeftEngine.SetDefault(false);
	if (PropulsionLeftEngine.SetIntervalBetweenUpdates(propulsionParams.delayLeftEngine) == 0)
	{
		if (PropulsionLeftEngine.SetRange(0, 180) == 0)
		{
			SerialLog.LogTV("["+ propulsionParams.id + "] Attached leftEngineControler to pin :", propulsionParams.leftEngineControlerPin);
			PropulsionLeftEngine.AttachToPin(propulsionParams.leftEngineControlerPin);			

			PropulsionRightEngine.SetId("Controler right engine");
			PropulsionRightEngine.SetDefault(false);
			if (PropulsionRightEngine.SetIntervalBetweenUpdates(propulsionParams.delayRightEngine) == 0)
			{
				if (PropulsionRightEngine.SetRange(0, 180) == 0)
				{
					SerialLog.LogTV("["+ propulsionParams.id + "] Attached rightEngineControler to pin :", propulsionParams.rightEngineControlerPin);
					PropulsionRightEngine.AttachToPin(propulsionParams.rightEngineControlerPin);	

					PropulsionDirection.SetId("Controler direction");
					PropulsionDirection.SetDefault(false);
					if (PropulsionDirection.SetIntervalBetweenUpdates(propulsionParams.delayDirection) == 0)
					{
						if (PropulsionDirection.SetRange(0, 180) == 0)
						{
							SerialLog.LogTV("[" + propulsionParams.id + "] Attached directionControler to pin :", propulsionParams.directionControlerPin);
							PropulsionDirection.AttachToPin(propulsionParams.directionControlerPin);

							Ok();
						}
					}
					else ErrorValueOutOfRange();
				}
				else ErrorValueOutOfRange();
			}
			else ErrorValueOutOfRange();
		}
		else ErrorValueOutOfRange();
	} 
	else  ErrorValueOutOfRange();

}


//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::SetEngineSpeed(int rightEngineSpeed, int leftEngineSpeed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionRightEngine.GoToPos(rightEngineSpeed) == 0)
	{
		if (PropulsionLeftEngine.GoToPos(leftEngineSpeed) == 0)
		{
			Ok();
		}
		else ErrorValueOutOfRange();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::SetDirection(int direction)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionDirection.GoToPos(direction) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionController::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	unChangement = false;
	PropulsionLeftEngineResult = PropulsionLeftEngine.Update();
	PropulsionRightEngineResult = PropulsionRightEngine.Update();
	PropulsionDirectionResult = PropulsionDirection.Update();
	if ((PropulsionLeftEngineResult == 1) || (PropulsionLeftEngineResult == -1)) unChangement = true;
	if ((PropulsionRightEngineResult == 1) || (PropulsionRightEngineResult == -1)) unChangement = true;
	if ((PropulsionDirectionResult == 1) || (PropulsionDirectionResult == -1)) unChangement = true;

	if (unChangement)
	{
		LedCommandeOk.SetState(LED_ON); 
	} else
	{
		if ((PropulsionLeftEngineResult != -2) && (PropulsionRightEngineResult != -2) && (PropulsionDirectionResult != -2))
		{ 
			LedCommandeOk.SetState(LED_OFF);
		}
	}
	LedCommandeOk.Update();
	LedCommandeError.Update();
}

//#endif
