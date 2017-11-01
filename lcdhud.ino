#include <LiquidCrystal.h>				//	LCD library
#include <OneWire.h>					//	One Wire library for DS sensors
#include <DallasTemperature.h>			//	Dallas Temperature Library
#include <Wire.h>						//	I2C Wire Library
#include "RTClib.h"						//	RTC library for DS1307

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
{
	lcd.setCursor(0, 1);
	lcd.print("Temp: ");
	lcd.print(temperature);
	
	sensors.requestTemperatures();
	temperature = sensors.getTempFByIndex(0);
}

void time()
{
	if (!rtc.begin()) {
		Serial.println("Couldn't find RTC");
		while (1);
	}

	if (!rtc.isrunning()) {
		Serial.println("RTC is NOT running!");
		rtc.adjust(DateTime(2017, 10, 31, 21, 12, 0));
	}
}

void lcdTime()
{
	lcd.setCursor(0, 0);

	DateTime now = rtc.now();

	lcd.print(now.hour(), DEC);
	lcd.print(':');
	lcd.print(now.minute(), DEC);
	lcd.print(':');
	lcd.print(now.second(), DEC);

	{
		//	this is corrects display error for minutes on lcd at turn of every minute
		if (now.minute() < 10) {
			lcd.setCursor(4, 0);
			lcd.print("0");
			lcd.print(now.minute(), DEC);
		}
		//	this is corrects display error for seconds on lcd at turn of every minute
		if (now.second() < 10) {
			lcd.setCursor(6, 0);
			lcd.print("0");
			lcd.print(now.second(), DEC);
		}
	}
	delay(500);
}


void setup(void)
{
	//	set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	Serial.begin(9600);

	time();

	sensors.begin();
}

void loop()
{
	serialDebug();
	temp();
	lcdTime();
}
