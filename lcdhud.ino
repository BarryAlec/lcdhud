#include <LiquidCrystal_I2C.h>			// I2C Library for LCD
#include <OneWire.h>					//	One Wire library for DS sensors
#include <DallasTemperature.h>			//	Dallas Temperature Library
#include <Wire.h>						//	I2C Wire Library
#include "RTClib.h"						//	RTC library for DS1307

// Versioning
byte version = 0;
byte subversion = 5;
byte build = 50;

byte batLeftFull[8] = {
	B11111,
	B10000,
	B10111,
	B10111,
	B10111,
	B10111,
	B10000,
	B11111,
};
byte batMidFull[8] = {
	B11111,
	B00000,
	B11111,
	B11111,
	B11111,
	B11111,
	B00000,
	B11111,
};
byte batRightFull[8] = {
	B11110,
	B00010,
	B11011,
	B11101,
	B11101,
	B11011,
	B00010,
	B11110,
};
byte batLeftLow[8] = {
	B11111,
	B10000,
	B10000,
	B10000,
	B10000,
	B10000,
	B10000,
	B11111,
};
byte batMidLow[8] = {
	B11111,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B11111,
};
byte batRightLow[8] = {
	B11110,
	B00010,
	B00011,
	B00001,
	B00001,
	B00011,
	B00010,
	B11110,
};
#define backlight_Button 18	// Pushbutton used to control lcd backlight

#define tempSens 10

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

#define NUM_A_SAMPLES 10	//	number of analog samples to take per reading

int sum = 0;	//	sum of samples taken
unsigned char sample_count = 0;	//	current sample number
float voltage = 0.0;	//	calculated voltage

unsigned int currentLcdLightOnTime = 0; // Current time LCD backlight has been on
unsigned long lcdLightOn_StartMillis;

unsigned int currentTempReading = 0; // Current temp reading

boolean isLcdLightOn;

int Con = 20;

float temperatureF = 0;
float temperatureC = 0;
int lowerLimit = 60;	//	Lower limit for temperature sensor
int upperLimit = 80;	//	Upper limit for temperature sensor

int buttonPushCounter = 0;
int buttonState = 0;
int lastButtonState = 0;

int lastState = 0;
int currentState = 0;

enum states {
	STOP, SENS, RTC, LCD, MAIN, CONT };
states state = STOP;

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
	displayHelp();

	//	set up the LCD's number of columns and rows:
	lcd.begin(20, 4);
	
	lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
	lcd.setBacklight(HIGH);

	isLcdLightOn = true;

	lcd.createChar(0, batLeftFull);
	lcd.createChar(1, batMidFull);
	lcd.createChar(2, batRightFull);
	lcd.createChar(3, batLeftLow);
	lcd.createChar(4, batMidLow);
	lcd.createChar(5, batRightLow);
	
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
	const unsigned long oneMinute = 1 * 60 * 1000UL;
	static unsigned long lastTempReading = 0 - oneMinute;

	unsigned long now = millis();

	if (now - lastTempReading >= oneMinute) {
		lastTempReading += oneMinute;
		temp();
	}
	lcdTime();
	backlightEnable();
	buttonDetect();
	tempLimit();
	voltageReference();
	//buttonIncrement();
	serialDebug();
}

//	SERIAL DEBUG MENU
void serialDebug()
{
	switch (getCommand())
	{
	case '1':
		state = SENS;
		break;
	case '2':
		state = RTC;
		break;
	case '3':
		state = LCD;
		break;
	case '0':
		state = MAIN;
		break;
	case 'c':
		state = CONT;
		break;
	default:
		break;
	}

	switch (state)
	{
	case SENS:
		tempDebug();
		break;
	case RTC:
		rtcStatus();
		break;
	case LCD:
		break;
	case MAIN:
		displayHelp();
		state = STOP;
		break;
	case STOP:
		break;
	default:
		break;
	}
}

char getCommand()
{
	char c = '\0';
	if (Serial.available())
	{
		c = Serial.read();
	}
	return c;
}

void displayHelp()
{
	//	This sets up the main menu for the serial debug menu
	String versionString;
	char buffer[16];
	sprintf(buffer, "Version %d.%02d.%02d", version, subversion, build);
	versionString = String(buffer);
	
	Serial.print(F("LCD HUD - "));
	Serial.print(versionString);
	Serial.println();
	Serial.println(F("\t1 - Query Temp Sensor"));
	Serial.println(F("\t2 - RTC Status"));
	Serial.println(F("\t3 - LCD Backlight Status"));
	Serial.println(F("\t0 - Main Menu"));
	Serial.println();
}

void tempDebug()
{
	//	Calls temp sensor and posts readings in serial menu
	sensors.requestTemperatures();
	temperatureF = sensors.getTempFByIndex(0);
	
	const unsigned long oneMinute = 1 * 60 * 1000UL;
	static unsigned long lastTempReading = 0 - oneMinute;

	unsigned long now = millis();

	if (now - lastTempReading >= oneMinute) {
		lastTempReading += oneMinute;
		Serial.print("Current reading from sensor: ");
		Serial.print(temperatureF);
		Serial.print('F');
		Serial.println();

	}
}

void rtcStatus()
{
	//	Calls RTC and posts time to serial menu
	rtc.begin();
	DateTime now = rtc.now();

	if (rtc.isrunning())
	{
		Serial.println("RTC is running.");
		Serial.println("Current time: ");
		Serial.print(now.hour(), DEC);
		Serial.print(':');
		Serial.print(now.minute(), DEC);
		Serial.print(':');
		Serial.print(now.second(), DEC);
	}
	else
	{
		Serial.println("RTC is not running.");
	}
}
void temp()
//	Function for requesting temperature from sensor and printing to LCD
{

		sensors.requestTemperatures();
		temperatureF = sensors.getTempFByIndex(0);
		lcd.setCursor(0, 1);
		lcd.print("Temp: ");
		lcd.print(round(temperatureF));
		lcd.print((char)223);
		lcd.print('F');

		temperatureC = sensors.getTempCByIndex(0);

		lcd.setCursor(12, 1);
		lcd.print(char(126));
		lcd.setCursor(15, 1);
		lcd.print(round(temperatureC));
		lcd.print((char)223);
		lcd.print('C');
	}

void tempLimit()
//	Triggers LED's when temperature reaches an upper and lower limit
{
	if (temperatureF >= upperLimit) {
		for (int fadeValue = 0; fadeValue <= 255; fadeValue += 15) {
			analogWrite(led_Red, fadeValue);
			delay(30);
		}
		for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 15) {
			analogWrite(led_Red, fadeValue);
			delay(30);
		}
	}

	if (temperatureF <= lowerLimit) {
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

	DateTime now = rtc.now();

	static byte oldSecond = 60;
	// display only if time changes

	if (now.second() != oldSecond) {
		lcd.setCursor(0, 0);

		print2digits(now.hour());
		lcd.print(':');
		print2digits(now.minute());
	}
		
	static byte oldDay = 99;
	// display only if day changes
	if (now.day() != oldDay) {
		lcd.setCursor(6, 0);

		lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);

		lcd.setCursor(10, 0);
		print2digits(now.year());
		lcd.print('/');
		print2digits(now.month());
		lcd.print('/');
		print2digits(now.day());
	}
}

void print2digits(int number)
// Adds a extra 0 for any number value < 10
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
}

void buttonDetect()
// Triggers a buzzer when button goes from high to low
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

void voltageReference()
//	Takes samples voltages from voltage divider and posts them to LCD
{
	const unsigned long oneMinute = 1 * 60 * 1000UL;
	static unsigned long lastVReading = 0 - oneMinute;

	unsigned long now = millis();

	if (now - lastVReading >= oneMinute) {
		lastVReading += oneMinute;
		while (sample_count < NUM_A_SAMPLES) {
			sum += analogRead(A2);
			sample_count++;
			delay(10);
		}
		voltage = ((float)sum / (float)NUM_A_SAMPLES * 4.906) / 1024.0;

		lcd.setCursor(0, 2);
		lcd.print(voltage * 1.1);
		lcd.print(" V");
		sample_count = 0;
		sum = 0;
	}
}