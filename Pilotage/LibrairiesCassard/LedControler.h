//#ifndef LedControler_h
//#define LedControler_h

#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>



// Led states supported:
enum LedState { LED_ON, LED_OFF };

class LedControler
{
    public:
        LedControler();
        void SetState(LedState state);
        void StartBlinking();
        void StopBlinking();
        void BlinkNTimes(int n);
        void SetBlinkingInterval(int blinkInterval);
        void SetDelayOff(unsigned long delayOff);
        void SetDelayOn(unsigned long delayOn);
        void AttachToPin(int ledPin);
        void Update();
		void SetDefault(boolean init);
		void SetId(String id);
private:
		String _id;
        LedState _state;
        unsigned long _lastTimeChange;
        boolean _blinking;
        int _ledPin;
		char _ledPinString[5];
		unsigned long _delayOff;
		unsigned long _delayOn;
        boolean _stopBlinking;
        int _blinkingTimes;
};

//---------------------------------------------------------------------------------------------------------------------------------------
LedControler::LedControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->SetDefault(true);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::SetDefault(boolean init)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (!init)
	{
		SerialLog.Log("["+ this->_id + " ("+ this->_ledPinString+ ")] Set Default");
		this->_ledPin = 0;
		itoa(this->_ledPin, this->_ledPinString, 10);
	}
	this->_state = LED_OFF;
	this->_blinking = false;
	this->_stopBlinking = false;
	this->_lastTimeChange = millis();
	this->_delayOff = 500;
	this->_delayOn = 500;
	this->_blinkingTimes = -1;
	digitalWrite(_ledPin, LOW);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::AttachToPin(int ledPin)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_ledPin = ledPin;
	itoa(this->_ledPin, this->_ledPinString, 10);
	pinMode(ledPin, OUTPUT);
	SerialLog.LogTV("["+ _id + "] Attached to pin :", ledPin);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::SetState(LedState state)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_blinking = false;
	this->_blinkingTimes = -1;
	this->_state = state;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::SetId(String id)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_id = id;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::StartBlinking()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_blinkingTimes = -1;
	this->_blinking = true;
	this->_stopBlinking = false;
	SerialLog.Log("["+ this->_id + " (" + this->_ledPinString + ")] Start Blinking");
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::StopBlinking()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	this->_blinking = false;
	this->_blinkingTimes = -1;
	this->_stopBlinking = false;
	this->_state = LED_OFF;
	digitalWrite(this->_ledPin, LOW);
	SerialLog.Log("["+ this->_id + " (" + this->_ledPinString + ")] Stop Blinking");
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::BlinkNTimes(int n)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SerialLog.LogTVT("["+ this->_id + " (" + this->_ledPinString + ")] Blink ",n," time(s)");
	this->_blinkingTimes = n;
	this->_stopBlinking = false;
	this->_blinking = true;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::SetBlinkingInterval(int interval)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SerialLog.LogTV("["+ this->_id + " (" + this->_ledPinString + ")] Set Blinking Interval ", interval);
	this->_delayOn = interval;
	this->_delayOff = interval;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::SetDelayOn(unsigned long delayOn)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SerialLog.LogTV("["+ this->_id + " (" + this->_ledPinString + ")] Set Delay On ", delayOn);
	this->_delayOn = delayOn;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::SetDelayOff(unsigned long delayOff)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	SerialLog.LogTV("["+ this->_id + " (" + this->_ledPinString + ")] Set Delay Off ", delayOff);
	this->_delayOff = delayOff;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LedControler::Update()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	unsigned long ms = millis();
	if (this->_blinking)
	{  
		SerialLog.Log("[" + _id + " (" + this->_ledPinString + ")] Blinking ...");
		if (this->_state == LED_OFF)
		{
		if ((ms - this->_lastTimeChange) > this->_delayOn) // time to go OFF
		{
			this->_lastTimeChange = ms;
			SerialLog.Log("[" + this->_id + " (" + this->_ledPinString + ")] Set pin OFF");
			digitalWrite(this->_ledPin, LOW);
			if (this->_stopBlinking)
			{
				this->_blinking = false;
			} else
			{
				this->_state = LED_ON;
			}  
		}
		} else
		{
			if ((ms - this->_lastTimeChange) > this->_delayOff) // time to go ON
			{
				_lastTimeChange = ms;
				SerialLog.Log("[" + this->_id + " (" + this->_ledPinString + ")] Set pin ON");
				digitalWrite(_ledPin, HIGH);
				this->_state = LED_OFF;

			// Testing the n times to stop blinking
			if (this->_blinkingTimes > 0)
			{
				this->_blinkingTimes -= 1;
			}
			if (this->_blinkingTimes == 0)
			{
				this->_stopBlinking = true;
			}
		}
		}
	} else
	{
		if (this->_state == LED_OFF)
		{
			SerialLog.Log("[" + this->_id + " (" + this->_ledPinString + ")] Set pin OFF");
			digitalWrite(this->_ledPin, LOW);
		} else
		{
			SerialLog.Log("[" + this->_id + " (" + this->_ledPinString + ")] Set pin ON");
			digitalWrite(this->_ledPin, HIGH);
		}
	}
}

//#endif
