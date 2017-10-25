#include <LiquidCrystal.h>

// Temp Sensor using a DS18S20 sensor connected via OneWire lib:
#include <OneWire.h>
#include <DallasTemperature.h>

// Date and time using a DS1307 RTC connected via I2C and Wire lib:
#include <Wire.h>
#include "RTClib.h"

/*
initialize the library by associating any needed LCD interfact pin
with the arduino pin number it is connected to:
*/
int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

int tempSens = 10;

float temperature = 0;

char daysOfTheWeek[7][24] = { "Sunday", "Monday", "Tuesday",
"Wednesday", "Thursday", "Friday", "Saturday" };

//Setup a oneWire instance to communicate with any OneWire devices:
OneWire oneWirePin(tempSens);

DallasTemperature sensors(&oneWirePin);

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

RTC_DS1307 rtc;

void temp()
{
	lcd.setCursor(0, 1);
	lcd.print("Temp: ");
	lcd.print(temperature);

	Serial.print("Requesting Temperatures from sensors: ");
	sensors.requestTemperatures();
	Serial.print("DONE");

	temperature = sensors.getTempFByIndex(0);

	Serial.print("Temp: ");
	Serial.print(temperature);
}

void time()
{
	if (!rtc.begin()) {
		Serial.println("Couldn't find RTC");
		while (1);
	}

	if (!rtc.isrunning()) {
		Serial.println("RTC is NOT running!");
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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

	delay(1000);
}


void setup(void)
{
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	Serial.begin(9600);

	time();

	sensors.begin();
}

void loop()
{
	temp();
	lcdTime();
}
