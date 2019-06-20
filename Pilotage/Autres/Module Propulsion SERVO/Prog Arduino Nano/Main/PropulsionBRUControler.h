//#ifndef PropulsionBRUControler_h
//#define PropulsionBRUControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LedControler.h"
#include "ServoControler.h"

struct PropulsionBRUParams
{
	String id;
	int okLedPin;
	int errorLedPin;
	int leftEngineControlerPin;
	int rightEngineControlerPin;
	int directionControlerPin;
	int leftEngineSpeed;
	int rightEngineSpeed;
	int directionSpeed;
};
PropulsionBRUParams propulsionBRUParams;

class PropulsionBRUControler
{
    public:
        PropulsionBRUControler();
		void Setup(PropulsionBRUParams propulsionBRUParams);
		void ForwardEngine(int engineSpeed);
		void ReverseEngine(int engineSpeed);
		void StopEngine();
		void TurnRight(int angle);
		void TurnLeft(int angle);		
		void GoStraight();
		void Update();
		void SetId(String id);
		void ErrorValueOutOfRange();
		void Ok();
	private:
		String _id;
		int _leftEngineSpeed;
		int _rightEngineSpeed;
		int _directionSpeed;
		bool _moving;
		int _propulsionRightEngineLastPos;
		int _propulsionLeftEngineLastPos;

		ServoControler PropulsionLeftEngine; // Création de l'objet servo pour la Propulsion du moteur Gauche
		ServoControler PropulsionRightEngine;// Création de l'objet servo pour la Propulsion du moteur Droit
		ServoControler PropulsionDirection;  // Création de l'objet servo pour la Direction

		LedControler _ledCommandeOk;          // Création de l'objet led verte pour commande acceptée
		LedControler _ledCommandeError;       // Création de l'objet led verte pour commande refusée
};

//---------------------------------------------------------------------------------------------------------------------------------------
PropulsionBRUControler::PropulsionBRUControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_leftEngineSpeed = 254;
	_rightEngineSpeed = 254;
	_directionSpeed = 254;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::ErrorValueOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_ledCommandeOk.SetState(LED_OFF);
	_ledCommandeError.SetDelayOn(50);
	_ledCommandeError.SetDelayOff(100);
	_ledCommandeError.BlinkNTimes(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::Ok()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_ledCommandeOk.SetState(LED_ON);
	_ledCommandeError.SetDelayOn(500);
	_ledCommandeError.SetDelayOff(500);
	_ledCommandeError.SetState(LED_OFF);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::Setup(PropulsionBRUParams propulsionBRUParams)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SetId(propulsionBRUParams.id);

	_ledCommandeOk.SetId("Led Ok");
	_ledCommandeOk.SetDefault(false);
	SerialLog.LogTV("["+ propulsionBRUParams.id+ "] Attached okLed to pin :", propulsionBRUParams.okLedPin);
	_ledCommandeOk.AttachToPin(propulsionBRUParams.okLedPin);					// attache la pin 2 à l'objet led LedCommandeOk

	_ledCommandeError.SetId("Led Error");
	_ledCommandeError.SetDefault(false);
	SerialLog.LogTV("["+ propulsionBRUParams.id + "] Attached errorLed to pin :", propulsionBRUParams.errorLedPin);
	_ledCommandeError.AttachToPin(propulsionBRUParams.errorLedPin);			// attache la pin 13 à l'objet led LedCommandeError

	PropulsionLeftEngine.SetId("Controler left engine");
	PropulsionLeftEngine.SetDefault(false);
	if (PropulsionLeftEngine.SetSpeed(propulsionBRUParams.leftEngineSpeed) == 0)
	{
		if (PropulsionLeftEngine.SetRange(0, 180) == 0)
		{
			SerialLog.LogTV("["+ propulsionBRUParams.id + "] Attached leftEngineControler to pin :", propulsionBRUParams.leftEngineControlerPin);
			PropulsionLeftEngine.AttachToPin(propulsionBRUParams.leftEngineControlerPin);			

			PropulsionRightEngine.SetId("Controler right engine");
			PropulsionRightEngine.SetDefault(false);
			if (PropulsionRightEngine.SetSpeed(propulsionBRUParams.rightEngineSpeed) == 0)
			{
				if (PropulsionRightEngine.SetRange(0, 180) == 0)
				{
					SerialLog.LogTV("["+ propulsionBRUParams.id + "] Attached rightEngineControler to pin :", propulsionBRUParams.rightEngineControlerPin);
					PropulsionRightEngine.AttachToPin(propulsionBRUParams.rightEngineControlerPin);	

					PropulsionDirection.SetId("Controler direction");
					PropulsionDirection.SetDefault(false);
					if (PropulsionDirection.SetSpeed(propulsionBRUParams.directionSpeed) == 0)
					{
						if (PropulsionDirection.SetRange(0, 180) == 0)
						{
							SerialLog.LogTV("[" + propulsionBRUParams.id + "] Attached directionControler to pin :", propulsionBRUParams.directionControlerPin);
							PropulsionDirection.AttachToPin(propulsionBRUParams.directionControlerPin);

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
void PropulsionBRUControler::ForwardEngine(int engineSpeed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int controlerPosition = 90 + ((engineSpeed * 90) / 100);
	bool noErrorRight = (PropulsionRightEngine.SetPosition(controlerPosition, false) == 0);
	bool noErrorLeft = (PropulsionLeftEngine.SetPosition(controlerPosition, false) == 0);
	if (noErrorRight || noErrorLeft)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::ReverseEngine(int engineSpeed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int controlerPosition = 90 - ((engineSpeed * 90) / 100);
	bool noErrorRight = (PropulsionRightEngine.SetPosition(controlerPosition, false) == 0);
	bool noErrorLeft = (PropulsionLeftEngine.SetPosition(controlerPosition, false) == 0);
	if (noErrorRight || noErrorLeft)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}
//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::StopEngine()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	bool noErrorRight = (PropulsionRightEngine.SetPosition(90, false) == 0);
	bool noErrorLeft = (PropulsionLeftEngine.SetPosition(90, false) == 0);
	if (noErrorRight || noErrorLeft)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::TurnRight(int angle)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int controlerPosition = 90 + ((angle * 90) / 100);
	if (PropulsionDirection.SetPosition(controlerPosition, false) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::TurnLeft(int angle)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int controlerPosition = 90 - ((angle * 90) / 100);
	if (PropulsionDirection.SetPosition(controlerPosition, false) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::GoStraight()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionDirection.SetPosition(90, false) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionBRUControler::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int position;
	this->_moving = false;

	position = this->PropulsionRightEngine.GetPosition();
	if (position != this->_propulsionRightEngineLastPos) this->_moving = true;
	this->_propulsionRightEngineLastPos = position;

	position = this->PropulsionLeftEngine.GetPosition();
	if (position != this->_propulsionLeftEngineLastPos) this->_moving = true;
	this->_propulsionLeftEngineLastPos = position;

	if (this->_moving) this->_ledCommandeOk.SetState(LED_ON); else this->_ledCommandeOk.SetState(LED_OFF);

	this->_ledCommandeOk.Update();
	this->_ledCommandeError.Update();
}

//#endif
