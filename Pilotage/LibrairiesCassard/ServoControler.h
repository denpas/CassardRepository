#ifndef ServoControler_h
#define ServoControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>

//#include "Servo.h"
#include "VarSpeedServo.h" 

const unsigned long MaxSpeed = 254;
const unsigned long MinSpeed = 0;
const int RangeMin = 0;
const int RangeMax = 180;

class ServoControler
{
    public:
        ServoControler();
        int SetPosition(int pos, bool sync);
		int GetPosition();
        void AttachToPin(int servoPin);
        void Detach();
        int SetRange(int minPos, int maxPos);
        int SetSpeed(int speed);
		void SetDefault(boolean init);
		void SetId(String id);
    private:
		String _id;
		VarSpeedServo _servo;
        int _posMinAllowed;
        int _posMaxAllowed;
        int _servoPin;
		unsigned long _speed;
};

//---------------------------------------------------------------------------------------------------------------------------------------
void ServoControler::SetDefault(boolean init)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (!init)
	{
		SerialLog.Log("["+ this->_id + "] Set Default");
		this->_servoPin = 0;
	}
	this->_posMinAllowed = 0;
	this->_posMaxAllowed = 180;
	this->_speed = MaxSpeed;
}
	
//---------------------------------------------------------------------------------------------------------------------------------------
ServoControler::ServoControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SetDefault(true);
}

//---------------------------------------------------------------------------------------------------------------------------------------
int ServoControler::SetPosition(int pos, bool sync)
//---------------------------------------------------------------------------------------------------------------------------------------
{
    if ((pos > this->_posMaxAllowed) || (pos < this->_posMinAllowed))
    {
		SerialLog.LogTV("["+ this->_id + "] Error position out of range :", pos);
		SerialLog.LogTVTV("["+ this->_id + "] pos must be between ", this->_posMinAllowed," and ", this->_posMaxAllowed);
		return -1;
    } else
    {
		this->_servo.write(pos, _speed, sync);
		return 0;
    }  
}

//---------------------------------------------------------------------------------------------------------------------------------------
int ServoControler::GetPosition()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	return this->_servo.read();
}

//---------------------------------------------------------------------------------------------------------------------------------------
int ServoControler::SetSpeed(int speed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if ((speed > MaxSpeed) || (speed < MinSpeed))
	{
		SerialLog.LogTV("["+ this->_id + "] Error speed out of range :", speed);
		SerialLog.LogTVTV("["+ this->_id + "] Error speed must be in [", MinSpeed, " : ", MaxSpeed);
		return -1;
	} else
	{
		SerialLog.LogTV("["+ this->_id + "] Speed Updated to :", speed);
		this->_speed = speed;
		return 0;
    }  
}

//---------------------------------------------------------------------------------------------------------------------------------------
int ServoControler::SetRange(int minPos,int maxPos)
//---------------------------------------------------------------------------------------------------------------------------------------
{
    if ((minPos < RangeMin) || (maxPos > RangeMax)) 
    {
		SerialLog.Log("["+ this->_id + "] Error Range out of range");
		SerialLog.LogTV("["+ this->_id + "] Range Min must be >= ", RangeMin);
		SerialLog.LogTV("["+ this->_id + "] Range Max must be <= ", RangeMax);
		return -1;
	}
    else
    {
		SerialLog.LogTV("["+ this->_id + "] Set Range min to : ", minPos);
		SerialLog.LogTV("["+ this->_id + "] Set Range max to : ", maxPos);
		this->_posMinAllowed = minPos;
		this->_posMaxAllowed = maxPos;
		return 0;
    }  
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ServoControler::AttachToPin(int servoPin)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_servoPin = servoPin;
	this->_servo.attach(_servoPin);
	SerialLog.LogTV("["+ this->_id + "] Attached to pin :", servoPin);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ServoControler::Detach()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_servo.detach();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ServoControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_id = id;
}

#endif
