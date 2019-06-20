//#ifndef PropulsionBrushlessBrushlessControler_h
//#define PropulsionBrushlessBrushlessControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LedControler.h"

struct PropulsionBrushlessParams
{
	String id;
	int okLedPin;
	int errorLedPin;
	int controlerLeftEnginePin;
	int controlerRightEnginePin;
	int servoDirectionPin;
	int delayLeftEngine;
	int delayRightEngine;
	int delayDirection;
};
PropulsionBrushlessParams PropulsionBrushlessParams;
bool unChangement;
int PropulsionBrushlessLeftEngineResult;
int PropulsionBrushlessRightEngineResult;
int PropulsionBrushlessDirectionResult;

class PropulsionBrushlessControler
{
    public:
        PropulsionBrushlessControler();
		void Setup(PropulsionBrushlessParams PropulsionBrushlessParams);
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

		ServoControler PropulsionBrushlessLeftEngine; // Création de l'objet servo pour la PropulsionBrushless du moteur Gauche
		ServoControler PropulsionBrushlessRightEngine;// Création de l'objet servo pour la PropulsionBrushless du moteur Droit
		ServoControler PropulsionBrushlessDirection;  // Création de l'objet servo pour la Direction
		LedControler LedCommandeOk;          // Création de l'objet led verte pour commande acceptée
		LedControler LedCommandeError;       // Création de l'objet led verte pour commande refusée
};

//---------------------------------------------------------------------------------------------------------------------------------------
PropulsionBrushlessControler::PropulsionBrushlessControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_delayLeftEngine = 0;
	_delayRightEngine = 0;
	_delayDirection = 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBrushlessControler::SetChangeSpeed(int delayLeftEngine, int delayRightEngine, int delayDirection)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_delayLeftEngine = delayLeftEngine;
	_delayRightEngine = delayRightEngine;
	_delayDirection = delayDirection;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBrushlessControler::ErrorValueOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	LedCommandeOk.SetState(LED_OFF);
	LedCommandeError.SetDelayOn(50);
	LedCommandeError.SetDelayOff(100);
	LedCommandeError.BlinkNTimes(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBrushlessControler::Ok()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	LedCommandeOk.SetState(LED_ON);
	LedCommandeError.SetDelayOn(500);
	LedCommandeError.SetDelayOff(500);
	LedCommandeError.SetState(LED_OFF);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBrushlessControler::Setup(PropulsionBrushlessParams PropulsionBrushlessParams)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SetId(PropulsionBrushlessParams.id);

	LedCommandeOk.SetId("Led Ok");
	LedCommandeOk.SetDefault(false);
	SerialLog.LogTV("["+ PropulsionBrushlessParams.id+ "] Attached okLed to pin :", PropulsionBrushlessParams.okLedPin);
	LedCommandeOk.AttachToPin(PropulsionBrushlessParams.okLedPin);					// attache la pin 2 à l'objet led LedCommandeOk

	LedCommandeError.SetId("Led Error");
	LedCommandeError.SetDefault(false);
	SerialLog.LogTV("["+ PropulsionBrushlessParams.id + "] Attached errorLed to pin :", PropulsionBrushlessParams.errorLedPin);
	LedCommandeError.AttachToPin(PropulsionBrushlessParams.errorLedPin);			// attache la pin 13 à l'objet led LedCommandeError

	PropulsionBrushlessLeftEngine.SetId("Controler left engine");
	PropulsionBrushlessLeftEngine.SetDefault(false);
	if (PropulsionBrushlessLeftEngine.SetIntervalBetweenUpdates(PropulsionBrushlessParams.delayLeftEngine) == 0)
	{
		if (PropulsionBrushlessLeftEngine.SetRange(0, 180) == 0)
		{
			SerialLog.LogTV("["+ PropulsionBrushlessParams.id + "] Attached controlerLeftEngine to pin :", PropulsionBrushlessParams.controlerLeftEnginePin);
			PropulsionBrushlessLeftEngine.AttachToPin(PropulsionBrushlessParams.controlerLeftEnginePin);			

			PropulsionBrushlessRightEngine.SetId("Controler right engine");
			PropulsionBrushlessRightEngine.SetDefault(false);
			if (PropulsionBrushlessRightEngine.SetIntervalBetweenUpdates(PropulsionBrushlessParams.delayRightEngine) == 0)
			{
				if (PropulsionBrushlessRightEngine.SetRange(0, 180) == 0)
				{
					SerialLog.LogTV("["+ PropulsionBrushlessParams.id + "] Attached controlerRightEngine to pin :", PropulsionBrushlessParams.controlerRightEnginePin);
					PropulsionBrushlessRightEngine.AttachToPin(PropulsionBrushlessParams.controlerRightEnginePin);	

					PropulsionBrushlessDirection.SetId("Controler direction");
					PropulsionBrushlessDirection.SetDefault(false);
					if (PropulsionBrushlessDirection.SetIntervalBetweenUpdates(PropulsionBrushlessParams.delayDirection) == 0)
					{
						if (PropulsionBrushlessDirection.SetRange(0, 180) == 0)
						{
							SerialLog.LogTV("[" + PropulsionBrushlessParams.id + "] Attached servoDirection to pin :", PropulsionBrushlessParams.servoDirectionPin);
							PropulsionBrushlessDirection.AttachToPin(PropulsionBrushlessParams.servoDirectionPin);

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
void PropulsionBrushlessControler::SetEngineSpeed(int rightEngineSpeed, int leftEngineSpeed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionBrushlessRightEngine.GoToPos(rightEngineSpeed) == 0)
	{
		if (PropulsionBrushlessLeftEngine.GoToPos(leftEngineSpeed) == 0)
		{
			Ok();
		}
		else ErrorValueOutOfRange();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBrushlessControler::SetDirection(int direction)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionBrushlessDirection.GoToPos(direction) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBrushlessControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBrushlessControler::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	unChangement = false;
	PropulsionBrushlessLeftEngineResult = PropulsionBrushlessLeftEngine.Update();
	PropulsionBrushlessRightEngineResult = PropulsionBrushlessRightEngine.Update();
	PropulsionBrushlessDirectionResult = PropulsionBrushlessDirection.Update();
	if ((PropulsionBrushlessLeftEngineResult == 1) || (PropulsionBrushlessLeftEngineResult == -1)) unChangement = true;
	if ((PropulsionBrushlessRightEngineResult == 1) || (PropulsionBrushlessRightEngineResult == -1)) unChangement = true;
	if ((PropulsionBrushlessDirectionResult == 1) || (PropulsionBrushlessDirectionResult == -1)) unChangement = true;

	if (unChangement)
	{
		LedCommandeOk.SetState(LED_ON); 
	} else
	{
		if ((PropulsionBrushlessLeftEngineResult != -2) && (PropulsionBrushlessRightEngineResult != -2) && (PropulsionBrushlessDirectionResult != -2))
		{ 
			LedCommandeOk.SetState(LED_OFF);
		}
	}
	LedCommandeOk.Update();
	LedCommandeError.Update();
}

//#endif
