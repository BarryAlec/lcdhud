#include <LiquidCrystal.h>				//	LCD library
#include <OneWire.h>					//	One Wire library for DS sensors
#include <DallasTemperature.h>			//	Dallas Temperature Library
#include <Wire.h>						//	I2C Wire Library
#include "RTClib.h"						//	RTC library for DS1307

// Versioning
byte version = 0;
byte subversion = 3;
byte build = 2;

/*
initialize the library by associating any needed LCD interfact pin
with the arduino pin number it is connected to:
*/
int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

int tempSens = 10;

float temperature = 0;

char daysOfTheWeek[7][24] = { "Sunday", "Monday", "Tuesday",
"Wednesday", "Thursday", "Friday", "Saturday" };

//	Setup a oneWire instance to communicate with any OneWire devices:
OneWire oneWirePin(tempSens);

DallasTemperature sensors(&oneWirePin);

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

RTC_DS1307 rtc;

void setup(void)
{
	//	set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	Serial.begin(9600);
	Wire.begin();

	startupMessage();

	time();

	sensors.begin();
}

void loop()
{
	serialDebug();
	lcdTime();
	temp();
}

void serialDebug()
{
	//	Serial debug for temperature sensor
	Serial.print("Requesting temperatures from sensors: ");
	sensors.requestTemperatures();
	Serial.print("DONE");
	Serial.println();

	Serial.print("Temp: ");
	Serial.print(temperature);
	Serial.println();
	Serial.print("-------------------");
	Serial.println();

	//	Serial debug for RTC
	DateTime now = rtc.now();

	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(" (");
	Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
	Serial.print(") ");
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();
}

void temp()
//	Function for requesting temperature from sensor and printing to LCD
{
	lcd.setCursor(0, 1);
	lcd.print("Temp: ");
	lcd.print(round(temperature));
	lcd.print((char)223);
	lcd.print('F');

	sensors.requestTemperatures();
	temperature = sensors.getTempFByIndex(0);
}

void time()
//	Starting, initializing and setting your time for the RTC
{
	rtc.begin();

	if (!rtc.isrunning()) {
		Serial.println("RTC is NOT running!");
	}
	//	adjust time here before compiling
	rtc.adjust(DateTime(2017, 11, 4, 9, 54, 12));
}


void lcdTime()
//	Printing and displaying time and date from the RTC onto the LCD
{
	lcd.setCursor(0, 0);

	DateTime now = rtc.now();

	lcd.print(now.hour(), DEC);
	lcd.print(':');
	lcd.print(now.minute(), DEC);

	{
		//	this corrects display for hours on lcd when hour value is less than 10
		if (now.hour() >= 0 && now.hour() < 10) {
			lcd.setCursor(0, 0);
			lcd.print('0');
			lcd.print(now.hour(), DEC);
			lcd.setCursor(2, 0);
			lcd.print(':');
			lcd.print(now.minute(), DEC);
		}
		//	this corrects display for minutes on lcd when minute value is less than 10
		if (now.minute() >= 0 && now.minute() < 10) {
			lcd.setCursor(3, 0);
			lcd.print('0');
			lcd.print(now.minute(), DEC);

		}
	}

	lcd.setCursor(6, 0);
	lcd.print(now.year(), DEC);
	lcd.print('/');
	lcd.print(now.month(), DEC);
	lcd.print('/');
	lcd.print(now.day(), DEC);

	{
		// this corrects display for days on lcd when day value is less than 10
		if (now.day() >= 0 && now.day() < 10) {
			lcd.setCursor(14, 0);
			lcd.print('0');
			lcd.print(now.day(), DEC);
		}

	}




	delay(500);
}

void startupMessage()
//	Print start up message to LCD
{
	String versionString;
	char buffer[16];
	sprintf(buffer, "Version %d.%02d.%02d", version, subversion, build);
	versionString = String(buffer);

	lcd.setCursor(0, 0);
	lcd.print("D.A.T.E. Display");
	lcd.setCursor(0, 1);
	lcd.print(versionString);
	delay(2000);
	lcd.clear();
}