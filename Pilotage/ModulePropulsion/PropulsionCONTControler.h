//#ifndef PropulsionCONTControler_h
//#define PropulsionCONTControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include "LedControler.h"
#include "ServoControler.h"
#include "ContMotorControler.h"

const int MinimumMotorPositionValue = 64;
const int MotorPositionAmplitude = 254 - MinimumMotorPositionValue;
const int DirectionAmplitude = 90;

struct PropulsionCONTParams
{
	String id;
	int okLedPin;
	int errorLedPin;

	int leftEngineDirectionPin;
	int leftEngineSpeedPin;

	int rightEngineDirectionPin;
	int rightEngineSpeedPin;

	int directionPin;

	int proportionalMode;
};
PropulsionCONTParams propulsionCONTParams;

class PropulsionCONTControler
{
    public:
        PropulsionCONTControler();
		void Setup(PropulsionCONTParams propulsionCONTParams);
		void ForwardEngine(int enginePower);
		void ReverseEngine(int enginePower);
		void StopEngine();
		void TurnRight(int angle);
		void TurnLeft(int angle);		
		void GoStraight();
		void ProportionalModule(int enginePower,
								int turningAngle,
								int proportionalMode,
								int* leftEnginePower,
								int* rightEnginePower);
		void ApplyProportionalPower(int leftEngineSpeed,
									int rightEngineSpeed);
		void Update();
		void SetId(String id);
		void SetProportionalMode(int proportionalMode);
		void ErrorValueOutOfRange();
		void Ok();
	private:
		String _id;
		int _proportionalMode;
		int _turningAngle;
		int _enginePower;

		ContMotorControler PropulsionLeftEngine;	// Création de l'objet servo pour la Propulsion du moteur Gauche
		ContMotorControler PropulsionRightEngine;	// Création de l'objet servo pour la Propulsion du moteur Droit
		ServoControler PropulsionDirection;			// Création de l'objet servo pour la Direction

		LedControler _ledCommandeOk;				// Création de l'objet led verte pour commande acceptée
		LedControler _ledCommandeError;				// Création de l'objet led verte pour commande refusée
};

//---------------------------------------------------------------------------------------------------------------------------------------
PropulsionCONTControler::PropulsionCONTControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::ErrorValueOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_ledCommandeOk.SetState(LED_OFF);
	_ledCommandeError.SetDelayOn(50);
	_ledCommandeError.SetDelayOff(100);
	_ledCommandeError.BlinkNTimes(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::Ok()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_ledCommandeOk.SetState(LED_ON);
	_ledCommandeError.SetDelayOn(500);
	_ledCommandeError.SetDelayOff(500);
	_ledCommandeError.SetState(LED_OFF);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::Setup(PropulsionCONTParams propulsionCONTParams)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SetId(propulsionCONTParams.id);

	_ledCommandeOk.SetId("Led Ok");
	_ledCommandeOk.SetDefault(false);
	SerialLog.LogTV("["+ propulsionCONTParams.id+ "] Attached okLed to pin :", propulsionCONTParams.okLedPin);
	_ledCommandeOk.AttachToPin(propulsionCONTParams.okLedPin);					// attache la pin 2 à l'objet led LedCommandeOk

	_ledCommandeError.SetId("Led Error");
	_ledCommandeError.SetDefault(false);
	SerialLog.LogTV("["+ propulsionCONTParams.id + "] Attached errorLed to pin :", propulsionCONTParams.errorLedPin);
	_ledCommandeError.AttachToPin(propulsionCONTParams.errorLedPin);			// attache la pin 13 à l'objet led LedCommandeError

	PropulsionLeftEngine.SetId("Controler left engine");
	PropulsionLeftEngine.SetDefault();

	PropulsionLeftEngine.AttachToPin(propulsionCONTParams.leftEngineSpeedPin, propulsionCONTParams.leftEngineDirectionPin);

	PropulsionRightEngine.SetId("Controler right engine");
	PropulsionRightEngine.SetDefault();
		
	PropulsionRightEngine.AttachToPin(propulsionCONTParams.rightEngineSpeedPin, propulsionCONTParams.rightEngineDirectionPin);
	PropulsionDirection.SetId("Controler direction");
	PropulsionDirection.SetDefault(false);

	if (PropulsionDirection.SetRange(0, 180) == 0)
	{
		SerialLog.LogTV("[" + propulsionCONTParams.id + "] Attached directionControler to pin :", propulsionCONTParams.directionPin);
		PropulsionDirection.AttachToPin(propulsionCONTParams.directionPin);

		Ok();
	}
	else ErrorValueOutOfRange();	

	SetProportionalMode(propulsionCONTParams.proportionalMode);

	_turningAngle = 0;
	_enginePower = 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::ProportionalModule(	int enginePower,
													int turningAngle,
													int proportionalMode,
													int* leftEnginePower,
													int* rightEnginePower)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	// enginePower is % from 0 to 100

	*leftEnginePower = enginePower;
	*rightEnginePower = enginePower;

	if (enginePower != 0)
	{
		*leftEnginePower = enginePower + (turningAngle * proportionalMode * 2 / enginePower);
		*rightEnginePower = enginePower - (turningAngle * proportionalMode * 2 / enginePower);
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::ApplyProportionalPower(	int leftEnginePower,
														int rightEnginePower)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	bool noErrorRight;
	bool noErrorLeft;

	// LEFT ENGINE
	if (leftEnginePower > 0)
	{
		noErrorLeft = (PropulsionLeftEngine.TurnLeft((leftEnginePower * MotorPositionAmplitude) / 100) == 0);
	}
	else
		if (leftEnginePower < 0)
		{
			noErrorLeft = (PropulsionLeftEngine.TurnRight((leftEnginePower * MotorPositionAmplitude) / 100) == 0);
		}
		else
		{
			noErrorLeft = (PropulsionLeftEngine.Stop() == 0);
		}

	// RIGHT ENGINE
	if (rightEnginePower > 0)
	{
		noErrorRight = (PropulsionRightEngine.TurnRight((rightEnginePower * MotorPositionAmplitude) / 100) == 0);
	}
	else
		if (rightEnginePower < 0)
		{
			noErrorRight = (PropulsionRightEngine.TurnLeft((rightEnginePower * MotorPositionAmplitude) / 100) == 0);
		}
		else
		{
			noErrorRight = (PropulsionRightEngine.Stop() == 0);
		}

	if (noErrorRight || noErrorLeft)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::ForwardEngine(int enginePower)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	// enginePower is % from 0 to 100
	// MotorPositionAmplitude is acceleration amplitude (ex:254 = 100%)

	bool noErrorRight;
	bool noErrorLeft;
	int	leftEnginePower;
	int	rightEnginePower;

	ProportionalModule(	enginePower,
						_turningAngle,
						_proportionalMode,
						&leftEnginePower,
						&rightEnginePower);
	
	ApplyProportionalPower(leftEnginePower, rightEnginePower);

	_enginePower = enginePower;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::ReverseEngine(int enginePower)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	bool noErrorRight;
	bool noErrorLeft;
	int	leftEnginePower;
	int	rightEnginePower;

	ProportionalModule(-enginePower,
						_turningAngle,
						_proportionalMode,
						&leftEnginePower,
						&rightEnginePower);

	ApplyProportionalPower(leftEnginePower, rightEnginePower);

	_enginePower = -enginePower;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::StopEngine()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	bool noErrorRight = (PropulsionRightEngine.Stop() == 0);
	bool noErrorLeft = (PropulsionLeftEngine.Stop() == 0);
	if (noErrorRight || noErrorLeft)
	{
		_enginePower = 0;
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::TurnRight(int angle)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int controlerPosition = DirectionAmplitude + angle;
	if (PropulsionDirection.SetPosition(controlerPosition, false) == 0)
	{
		int	leftEnginePower;
		int	rightEnginePower;

		_turningAngle = angle;

		ProportionalModule(	_enginePower,
							_turningAngle,
							_proportionalMode,
							&leftEnginePower,
							&rightEnginePower);

		ApplyProportionalPower(	leftEnginePower, rightEnginePower);

		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::TurnLeft(int angle)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int controlerPosition = DirectionAmplitude - angle;
	if (PropulsionDirection.SetPosition(controlerPosition, false) == 0)
	{
		int	leftEnginePower;
		int	rightEnginePower;

		_turningAngle = -angle;

		ProportionalModule(_enginePower,
			_turningAngle,
			_proportionalMode,
			&leftEnginePower,
			&rightEnginePower);

		ApplyProportionalPower(leftEnginePower, rightEnginePower);

		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::GoStraight()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (PropulsionDirection.SetPosition(90, false) == 0)
	{
		int	leftEnginePower;
		int	rightEnginePower;

		_turningAngle = 0;

		ProportionalModule(	_enginePower,
							_turningAngle,
							_proportionalMode,
							&leftEnginePower,
							&rightEnginePower);

		ApplyProportionalPower(leftEnginePower, rightEnginePower);

		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::SetProportionalMode(int proportionalMode)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_proportionalMode = proportionalMode;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void PropulsionCONTControler::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_ledCommandeOk.Update();
	this->_ledCommandeError.Update();
}

//#endif
