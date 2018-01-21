#include <LiquidCrystal_I2C.h>			// I2C Library for LCD
#include <OneWire.h>					//	One Wire library for DS sensors
#include <DallasTemperature.h>			//	Dallas Temperature Library
#include <Wire.h>						//	I2C Wire Library
#include "RTClib.h"						//	RTC library for DS1307

// Versioning
byte version = 0;
byte subversion = 5;
byte build = 15;
#define backlight_Button 18	// Pushbutton used to control lcd backlight


#define BUZZER_PIN 11
#define led_Red 13
#define led_Blue 9

// Defining LCD
#define LCD_I2C_ADDR 0x27

#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

#define LCD_LIGHT_ON_TIME 60000 // How long (in ms) the LCD should stay on

unsigned int currentLcdLightOnTime = 0; // Current time LCD backlight has been on
unsigned long lcdLightOn_StartMillis;

boolean isLcdLightOn;


int Con = 20;

#define tempSens 10

float temperature = 0;
int lowerLimit = 60;	//	Lower limit for temperature sensor
int upperLimit = 80;	//	Upper limit for temperature sensor

int buttonPushCounter = 0;
int buttonState = 0;
int lastButtonState = 0;

char daysOfTheWeek[7][24] = { "Sun", "Mon", "Tue",
"Wed", "Thu", "Fri", "Sat" };

//	Setup a oneWire instance to communicate with any OneWire devices:
OneWire oneWirePin(tempSens);

DallasTemperature sensors(&oneWirePin);

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

RTC_DS1307 rtc;

void setup(void)
{
	Serial.begin(115200);
	//	set up the LCD's number of columns and rows:
	lcd.begin(20, 4);
	
	lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
	lcd.setBacklight(HIGH);

	isLcdLightOn = true;

	Wire.begin();

	analogWrite(6, Con);

	pinMode(backlight_Button, INPUT);
	pinMode(led_Red, OUTPUT);
	pinMode(led_Blue, OUTPUT);
	pinMode(BUZZER_PIN, OUTPUT);

	attachInterrupt(digitalPinToInterrupt(4), buttonDetect, FALLING);

	startupMessage();

	time();

	sensors.begin();
}

void loop()
{
	serialDebug();
	lcdTime();
	backlightEnable();
	buttonDetect();
	temp();
	tempLimit();
	//buttonIncrement();
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
			analogWrite(led_Red, fadeValue);
			delay(30);
		}
		for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 15) {
			analogWrite(led_Red, fadeValue);
			delay(30);
		}
	}

	if (temperature <= lowerLimit) {
		for (int fadeValue = 0; fadeValue <= 255; fadeValue += 15) {
			analogWrite(led_Blue, fadeValue);
			delay(30);
		}
		for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 15) {
			analogWrite(led_Blue, fadeValue);
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

	lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);

	lcd.setCursor(10, 0);
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

void buttonIncrement()
{
	buttonState = digitalRead(backlight_Button);
	
	buttonState != lastButtonState; {

		if (buttonState == LOW) {
			buttonPushCounter++;
			Serial.println("on");
			Serial.print("number of button pushes: ");
			Serial.println(buttonPushCounter);
		}
		else
		{
			Serial.println("off");
		}
		delay(50);
	}
}

void backlightEnable()
/* Checks how long LCD backlight has been on. If >= 60sec, backlight powers off. 
Triggering pushbutton enables backlight and resets timer for auto-off function.
*/
{
	buttonState = digitalRead(backlight_Button);

	if (buttonState == LOW) {
		Serial.println("Backlight On");

		lcdLightOn_StartMillis = millis();
		currentLcdLightOnTime = 0;
		isLcdLightOn = true;
		lcd.setBacklight(BACKLIGHT_ON);
	}
	else {
		if (isLcdLightOn) {
			currentLcdLightOnTime = millis() - lcdLightOn_StartMillis;
			if (currentLcdLightOnTime > LCD_LIGHT_ON_TIME) {
				isLcdLightOn = false;
				lcd.setBacklight(BACKLIGHT_OFF);
			}
		}
	}
		Serial.print("Backlight on time: ");
		Serial.println(currentLcdLightOnTime / 1000);
}

void buttonDetect()
{
	buttonState = digitalRead(backlight_Button);

	if (buttonState == LOW) {
		digitalWrite(BUZZER_PIN, HIGH);
		delay(50);
	}
	else {
		digitalWrite(BUZZER_PIN, LOW);
		delay(50);
	}
}