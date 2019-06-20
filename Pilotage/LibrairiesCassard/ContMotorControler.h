#ifndef ContMotorControler_h
#define ContMotorControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>

class ContMotorControler
{
    public:
        ContMotorControler();
        int TurnLeft(int speed);
		int TurnRight(int speed);
		int Stop();
		void AttachToPin(int ContMotorSpeedPin, int ContMotorDirectionPin);
		void SetDefault();
		void SetId(String id);
    private:
		String _id;
		int _posMinAllowed;
        int _posMaxAllowed;
		int _ContMotorDirectionPin;
		int _ContMotorSpeedPin;		
};

//---------------------------------------------------------------------------------------------------------------------------------------
void ContMotorControler::SetDefault()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_posMinAllowed = 0;
	this->_posMaxAllowed = 254;
}
	
//---------------------------------------------------------------------------------------------------------------------------------------
ContMotorControler::ContMotorControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SetDefault();
}

//---------------------------------------------------------------------------------------------------------------------------------------
int ContMotorControler::TurnLeft(int speed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SerialLog.LogTV("["+ _id + "] Turn left at speed :", speed);
	
	digitalWrite(this->_ContMotorDirectionPin, LOW);  
	analogWrite(this->_ContMotorSpeedPin, speed);

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
int ContMotorControler::Stop()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SerialLog.Log("["+ _id + "] Stop");

	digitalWrite(this->_ContMotorSpeedPin, 0); 

	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------------
int ContMotorControler::TurnRight(int speed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SerialLog.LogTV("[" + _id + "] Turn right at speed :", speed);

	digitalWrite(this->_ContMotorDirectionPin, HIGH);  

	analogWrite(this->_ContMotorSpeedPin, speed);

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ContMotorControler::AttachToPin(int ContMotorSpeedPin, int ContMotorDirectionPin)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_ContMotorSpeedPin = ContMotorSpeedPin;
	pinMode(ContMotorSpeedPin, OUTPUT);

	this->_ContMotorDirectionPin = ContMotorDirectionPin;
	pinMode(ContMotorDirectionPin, OUTPUT);

	SerialLog.LogTV("[" + _id + "] Attached <speed> to pin :", ContMotorSpeedPin);
	SerialLog.LogTV("[" + _id + "] Attached <direction1> to pin :", ContMotorDirectionPin);
}


//---------------------------------------------------------------------------------------------------------------------------------------
void ContMotorControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	_id = id;
}

#endif
