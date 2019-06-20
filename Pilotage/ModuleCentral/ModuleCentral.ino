#include <Wire.h>
#include "SPI.h"
#include "Phpoc.h"
#include "Protocol.h"

PhpocServer server(80);

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);
  server.beginWebSocket("_Cassard");
}

int moduleId = 2;
String trame;
int i;

String getStringPartByNr(String data, char separator, int index)
{
	// spliting a string and return the part nr index
	// split by separator

	int stringData = 0;        //variable to count data part nr 
	String dataPart = "";      //variable to hole the return text

	for (int i = 0; i<data.length() - 1; i++) {    //Walk through the text one letter at a time

		if (data[i] == separator) {
			//Count the number of times separator character appears in the text
			stringData++;

		}
		else if (stringData == index) {
			//get the text when separator is the rignt one
			dataPart.concat(data[i]);

		}
		else if (stringData>index) {
			//return text and stop if the next separator appears - to save CPU-time
			return dataPart;
			break;

		}

	}
	//return text if this is the last part
	return dataPart;
}

void loop() 
{
	PhpocClient client = server.available();

	if (client) {
		trame = client.readLine();
		Wire.beginTransmission(moduleId);
		for (i = 0; i<4; i++) {
			Wire.write(char(getStringPartByNr(trame, ',', i).toInt()));
		}
		Wire.endTransmission();
	}
	/*
	delay(1000);
	delayInterCommandeMS = 2000;

	for (proportionalMode = 0; proportionalMode < 6; proportionalMode++)
	{
		sendTrameToI2C("2,18," + String(proportionalMode, DEC) + ",255");			// Mode proportionnel de 0 ? 5
		sendTrameToI2C("2,17,255");				// GoStraight
		sendTrameToI2C("2,12,30,255");			// Vitesse 30*

		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,15,30,255");			// droite de 30
		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,15,60,255");			// droite de 60
		delay(delayInterCommandeMS);			// 10s

		sendTrameToI2C("2,17,255");				// GoStraight
		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,16,30,255");			// gauche de 30
		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,16,60,255");			// gauche de 60
		delay(delayInterCommandeMS);			// 10s

		sendTrameToI2C("2,17,255");				// GoStraight
		delay(delayInterCommandeMS);			// 10s

												// MEME SEQUENCE MAIS AVEC VITESSE MAXI

		sendTrameToI2C("2,12,100,255");			// Vitesse 100%

		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,15,30,255");			// droite de 30
		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,15,60,255");			// droite de 60
		delay(delayInterCommandeMS);			// 10s

		sendTrameToI2C("2,17,255");				// GoStraight
		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,16,30,255");			// gauche de 30
		delay(delayInterCommandeMS);			// 10s
		sendTrameToI2C("2,16,60,255");			// gauche de 60
		delay(delayInterCommandeMS);			// 10s

		sendTrameToI2C("2,17,255");				// GoStraight
		delay(delayInterCommandeMS);			// 10s

		sendTrameToI2C("2,14,255");				// Stop
		delay(2000);							// 20s
	}
	*/
}
