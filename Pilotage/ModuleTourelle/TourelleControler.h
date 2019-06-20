//#ifndef TourelleControler_h
//#define TourelleControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>
#include <LedControler.h>
#include "ServoControler.h"

struct TourelleParams
{
	String id;
	int okLedPin;
	int errorLedPin;
	int servoAzimutPin;
	int servoElevationPin;
	int smokePin;
	int soundPin;
	int azimutSpeed;
	int elevationSpeed;
};

bool unServoBouge;

class TourelleControler
{
    public:
        TourelleControler();
		void Setup(TourelleParams tourelleParams);
		void GoToPos(int azimut, int elevation);
        void Update();
		void SetId(String id);
		void SetSpeed(int azimutSpeed, int elevationSpeed);
		void ErrorValueOutOfRange();
		void Ok();
	private:
		String _id;
		bool _moving;
		int _azimutLastPos;
		int _elevationLastPos;
		int _azimutSpeed;
		int _elevationSpeed;

		ServoControler _TourelleAzimut;       // Cr�ation de l'objet servo pour mouvement azimut de la tourelle du canon de 100mm
		ServoControler _TourelleElevation;    // Cr�ation de l'objet servo pour mouvement d'�l�vation de la tourelle du canon de 100mm
		
		LedControler _LedCommandeOk;          // Cr�ation de l'objet led verte pour commande accept�e
		LedControler _LedCommandeError;       // Cr�ation de l'objet led verte pour commande refus�e
};

//---------------------------------------------------------------------------------------------------------------------------------------
TourelleControler::TourelleControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{

}

//---------------------------------------------------------------------------------------------------------------------------------------
void TourelleControler::SetSpeed(int azimutSpeed, int elevationSpeed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_azimutSpeed = azimutSpeed;
	this->_TourelleAzimut.SetSpeed(azimutSpeed);
	this->_elevationSpeed = elevationSpeed;
	this->_TourelleElevation.SetSpeed(elevationSpeed);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void TourelleControler::ErrorValueOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_LedCommandeOk.SetState(LED_OFF);
	this->_LedCommandeError.SetDelayOn(50);
	this->_LedCommandeError.SetDelayOff(100);
	this->_LedCommandeError.BlinkNTimes(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void TourelleControler::Ok()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_LedCommandeOk.SetState(LED_ON);
	this->_LedCommandeError.SetDelayOn(500);
	this->_LedCommandeError.SetDelayOff(500);
	this->_LedCommandeError.SetState(LED_OFF);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void TourelleControler::Setup(TourelleParams tourelleParams)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->SetId(tourelleParams.id);

	this->_LedCommandeOk.SetId("Led Ok");
	this->_LedCommandeOk.SetDefault(false);
	SerialLog.LogTV("["+ tourelleParams.id+ "] Attached okLed to pin :", tourelleParams.okLedPin);
	this->_LedCommandeOk.AttachToPin(tourelleParams.okLedPin);					// attache la pin 2 � l'objet led LedCommandeOk

	this->_LedCommandeError.SetId("Led Error");
	this->_LedCommandeError.SetDefault(false);
	SerialLog.LogTV("["+ tourelleParams.id + "] Attached errorLed to pin :", tourelleParams.errorLedPin);
	this->_LedCommandeError.AttachToPin(tourelleParams.errorLedPin);			// attache la pin 13 � l'objet led LedCommandeError

	this->_TourelleAzimut.SetId("Servo Azimut");
	this->_TourelleAzimut.SetDefault(false);
	if (this->_TourelleAzimut.SetSpeed(tourelleParams.azimutSpeed) == 0)
	{
		if (this->_TourelleAzimut.SetRange(0, 180) == 0)
		{
			SerialLog.LogTV("["+ tourelleParams.id + "] Attached servoAzimut to pin :", tourelleParams.servoAzimutPin);
			this->_TourelleAzimut.AttachToPin(tourelleParams.servoAzimutPin);			// attache la pin 9 � l'objet servo TourelleAzimut

			this->_TourelleElevation.SetId("Servo Elevation");
			this->_TourelleElevation.SetDefault(false);
			if (this->_TourelleElevation.SetSpeed(tourelleParams.elevationSpeed) == 0)
			{
				if (this->_TourelleElevation.SetRange(0, 60) == 0)
				{
					SerialLog.LogTV("["+ tourelleParams.id + "] Attached servoElevation to pin :", tourelleParams.servoElevationPin);
					this->_TourelleElevation.AttachToPin(tourelleParams.servoElevationPin);	// attache la pin 10 � l'objet servo TourelleElevation

					SerialLog.LogTVT("["+ tourelleParams.id + "] Attached smoke to pin :", tourelleParams.smokePin, " NOT YET IMPLEMENTED");
					// � faire
					SerialLog.LogTVT("["+ tourelleParams.id + "] Attached sound to pin :", tourelleParams.soundPin, " NOT YET IMPLEMENTED");
					// � faire

					Ok();
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
void TourelleControler::GoToPos(int azimut, int elevation)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (this->_TourelleElevation.SetPosition(elevation, false) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();

	if (this->_TourelleAzimut.SetPosition(azimut,false) == 0)
	{
		Ok();
	}
	else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void TourelleControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void TourelleControler::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	int position;
	this->_moving = false;

	position = this->_TourelleElevation.GetPosition();
	if (position != this->_elevationLastPos) this->_moving = true;
	this->_elevationLastPos = position;

	position = this->_TourelleAzimut.GetPosition();
	if (position != this->_azimutLastPos) this->_moving = true;
	this->_azimutLastPos = position;

	if (this->_moving) this->_LedCommandeOk.SetState(LED_ON); else this->_LedCommandeOk.SetState(LED_OFF);

	this->_LedCommandeOk.Update();
	this->_LedCommandeError.Update();
	//Smoke.Update();
	//Sound.Update();
}

//#endif
