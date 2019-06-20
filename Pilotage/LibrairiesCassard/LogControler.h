#include <C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h>

class LogControler
{
    public:
		bool suspended;
        LogControler();
		void Log(String title);
		void LogTV(String title, int value);
		void LogTVT(String title1, int value, String title2);
		void LogTVTV(String title1, int value1, String title2, int value2);
		void LogTVTVT(String title1, int value1, String title2, int value2, String title3);
		void SetSpeed(int speed);
		void Suspend();
    private:
        int _speed;
};

LogControler SerialLog;

//---------------------------------------------------------------------------------------------------------------------------------------
LogControler::LogControler()
//---------------------------------------------------------------------------------------------------------------------------------------
{
	suspended = true;
	if (suspended) return;
	SetSpeed(9600);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LogControler::SetSpeed(int speed)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (suspended) return;
	_speed = speed;
	Serial.begin(_speed); 
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LogControler::Log(String title)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (suspended) return;
	Serial.println(title);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LogControler::LogTV(String title, int value)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (suspended) return;
	Serial.print(title);
	Serial.println(value);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LogControler::LogTVT(String title1, int value,String title2)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (suspended) return;
	Serial.print(title1);
	Serial.print(value);
	Serial.println(title2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LogControler::LogTVTV(String title1, int value1, String title2, int value2)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (suspended) return;
	Serial.print(title1);
	Serial.print(value1);
	Serial.print(title2);
	Serial.println(value2);
}

//---------------------------------------------------------------------------------------------------------------------------------------
void LogControler::LogTVTVT(String title1, int value1, String title2, int value2, String title3)
//---------------------------------------------------------------------------------------------------------------------------------------
{
	if (suspended) return;
	Serial.print(title1);
	Serial.print(value1);
	Serial.print(title2);
	Serial.print(value2);
	Serial.println(title3);
}
