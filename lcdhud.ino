#include <LiquidCrystal.h>				//	LCD library
#include <OneWire.h>					//	One Wire library for DS sensors
#include <DallasTemperature.h>			//	Dallas Temperature Library
#include <Wire.h>						//	I2C Wire Library
#include "RTClib.h"						//	RTC library for DS1307

// Versioning
byte version = 0;
byte subversion = 4;
byte build = 6;

#define menuButton 18
#define ledRed 13
#define ledBlue 9

int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;	//	initializes library with assigned LCD pins

#define tempSens 10

float temperature = 0;
int lowerLimit = 60;	//	Lower limit for temperature sensor
int upperLimit = 80;	//	Upper limit for temperature sensor

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

	pinMode(menuButton, INPUT);
	pinMode(ledRed, OUTPUT);
	pinMode(ledBlue, OUTPUT);

	startupMessage();

	time();

	sensors.begin();
}

void loop()
{
	serialDebug();
	lcdTime();
	temp();
	tempLimit();
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

void tempLimit()
//	Triggers LED's when temperature reaches an upper and lower limit
{
	if (temperature >= upperLimit) {
		for (int fadeValue = 0; fadeValue <= 255; fadeValue += 15) {
			analogWrite(ledRed, fadeValue);
			delay(30);
		}
		for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 15) {
			analogWrite(ledRed, fadeValue);
			delay(30);
		}
	}

	if (temperature <= lowerLimit) {
		for (int fadeValue = 0; fadeValue <= 255; fadeValue += 15) {
			analogWrite(ledBlue, fadeValue);
			delay(30);
		}
		for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 15) {
			analogWrite(ledBlue, fadeValue);
			delay(30);
		}
	}
}

void time()
//	Starting, initializing and setting your time for the RTC
{
	rtc.begin();

	if (!rtc.isrunning()) {
		Serial.println("RTC is NOT running!");
	}
	//	following line sets time to computer's time at time of compiling
	//	remove comment tag in line below to set time
	//	rtc.adjust(DateTime(__DATE__, __TIME__));
}


void lcdTime()
//	Printing and displaying time and date from the RTC onto the LCD
{
	lcd.setCursor(0, 0);

	DateTime now = rtc.now();

	print2digits(now.hour());
	lcd.print(':');
	print2digits(now.minute());

	

	lcd.setCursor(6, 0);
	print2digits(now.year());
	lcd.print('/');
	print2digits(now.month());
	lcd.print('/');
	print2digits(now.day());

	delay(500);
}

void print2digits(int number)
{
	if (number >= 0 && number < 10) {
		lcd.print('0');
	}
	lcd.print(number, DEC);
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