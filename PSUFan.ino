/*
    Name:       PSUFan.ino
    Created:	21/10/2018 11:10:19
    Author:     Nigel Morton
*/
#include "PSUFan.h"
#include "eeprom_util.h"
#include "WiFiSettings.h"

const char* version = "V1.0";

DebugWeb debugWeb;
Ticker tmrReadSensor;
extern AsyncEventSource events; // event source (Server-Sent events)
bool shouldReboot = false;
bool fanState = false;
int fanSpeed;
bool manual = false;
String ntpTime = "";
bool bReadSensor = true;
int adc;
int temperature = 0;
int fanOnTemp;
int fanOffTemp;


//Read the sensors
void readSensor() { bReadSensor = true; }

void setup()
{
	char result[16];

	Serial.begin(115200);
	turnFan(FAN_OFF);
	pinMode(FAN, OUTPUT);      // sets the digital pin as output

	DEBUG_PRINTLN("");
	DEBUG_PRINTLN("Startup");

	WiFi.mode(WIFI_STA);	//WIFI_STA
	WiFi.hostname(hostName);

#ifdef STATIC
	WiFi.config(staticIP, subnet, gateway, dns);
#endif

	WiFi.begin(ssid, password);	

	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.printf("WiFi Failed!\n");
		debugWeb.wifiActive = false;
	}

	DEBUG_PRINTLN("WiFi connected!");
	sprintf(result, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
	DEBUG_PRINT(result);
	DEBUG_PRINTLN((char*)version);
	loadSettings();
	setupOTAUpdate();
	SPIFFS.begin();
	setupWebserver();

	//Start timers for system functions
	tmrReadSensor.attach(5, readSensor);		//Start timer to kick off sensor reading
	analogWriteFreq(2000);
}

// Main loop() function
void loop()
{
	char status[20];

	ArduinoOTA.handle();

	if (bReadSensor) {
		readADCSensor();
		bReadSensor = false;
		sprintf(status, "%d", temperature);
		events.send(status, "temp");
		//if ((temperature >= fanOnTemp) && (!fanState))
		if (temperature >= fanOnTemp)
			{
			//turnFan(FAN_ON);
			int temp = temperature - fanOnTemp;
			switch (temp) {
			case 0:
			case 1:
			case 2:
			case 3:
				turnFan(500);
				break;
			case 4:
			case 5:
			case 6:
				turnFan(700);
				break;
			case 7:
			case 8:
			case 9:
				turnFan(900);
				break;
			default:
				turnFan(1023);
				break;
			}
		}
		if ((temperature <= fanOffTemp) && (fanState))
			turnFan(0);
	}

	if (shouldReboot) {
		DEBUG_PRINTLN("Rebooting...");
		delay(100);
		ESP.restart();
	}
}

//Turn fan on/off (pwm)
void turnFan(int fSpeed) {
	char status[20];

	DEBUG_PRINTLN("turnFan");
	analogWrite(FAN, fSpeed);
	fanSpeed = fSpeed;
	fanState = fSpeed == 0 ? false : true;
	sprintf(status, "%d", fSpeed);
	events.send(status, "fan");
}

void readADCSensor() {
	//Read from ESP analogue pin
	adc = 1024 - analogRead(A0);
	temperature = convertToTemperature(adc, 1024.0);
	DEBUG_PRINTST("Raw %d: %d\n", adc , temperature);
}

#define RESISTANCE 10000
int convertToTemperature(int rawADC, double convertFactor)
{
	double Temp, tmp1;

	Temp = log(RESISTANCE*((convertFactor / rawADC - 1))); //When using PCF8591
	Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp))* Temp);
	Temp = Temp - 273.15;            // Convert Kelvin to Celcius
	return (int)Temp;
}

void loadSettings() {
	char result[16];

	DEBUG_PRINTLN("Eprom init");
	EEPROM.begin(32);
	eeprom_read_string(E_INIT, result, 5);
	if (strcmp(result, "INIT") != 0) {
		//EEPROM not initialised so set default values
		resetParameters();
	}

	fanOnTemp = eeprom_read_int(E_FANON);
	fanOffTemp = eeprom_read_int(E_FANOFF);
}

void saveSettings() {
	eeprom_write_int(E_FANON, fanOnTemp);
	eeprom_write_int(E_FANOFF, fanOffTemp);
	EEPROM.commit();
}

//Set default values
void resetParameters() {
	DEBUG_PRINTLN("EEPROM Init");
	eeprom_erase_all(0x00);
	EEPROM.commit();
	eeprom_write_int(E_FANON, 40);
	eeprom_write_int(E_FANOFF,30);
	eeprom_write_string(E_INIT, "INIT");
	EEPROM.commit();
}

// Build the XML file with settings for setup page
String buildSettingsXML()
{
	String strHTML;
	char buf[50];

	strHTML = "<?xml version = \"1.0\" ?><inputs><dOn>";
	sprintf(buf, "%d", fanOnTemp);
	strHTML += buf;
	strHTML += "</dOn><dOff>";
	sprintf(buf, "%d", fanOffTemp);
	strHTML += buf;
	strHTML += "</dOff></inputs>";
	return strHTML;
}
