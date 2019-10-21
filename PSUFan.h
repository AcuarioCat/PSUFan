#pragma once
#ifndef _PSUFan_H_
#define _PSUFan_H_

#include <ArduinoOTA.h>
#include "ESPAsyncWebServer.h"
#include <Ticker.h>
#include <EEPROM.h>
#include "debugWeb.h"
#include "AsyncWebServer.h"

/* Sensors */
constexpr auto FAN = 14;
constexpr auto NTC = 0;

constexpr auto FAN_ON = 1;
constexpr auto FAN_OFF = 0;

//ESP8266 int = 4 bytes
constexpr auto E_INIT = 20;				//Set to INIT after first power on. First power on will initialise all values to default, subsequent will not.
constexpr auto E_FANON = 0;
constexpr auto E_FANOFF = 4;

extern void resetParameters();
extern String buildSettingsXML();
extern void turnFan(int);
extern void saveSettings();
extern bool shouldReboot;
extern int fanOnTemp;
extern int fanOffTemp;
extern void sendSettings();

extern const char * otaPassword;

static const char* ssdpTemplate =
"<?xml version=\"1.0\"?>"
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
"<specVersion>"
"<major>1</major>"
"<minor>0</minor>"
"</specVersion>"
"<URLBase>http://%d.%d.%d.%d:80/description.xml</URLBase>"
"<device>"
"<deviceType>urn:schemas-upnp-org:device:BinaryLight:1</deviceType>"
"<friendlyName>%s</friendlyName>"
"<presentationURL>http://%d.%d.%d.%d</presentationURL>"
"<serialNumber>%u</serialNumber>"
"<modelName>%s</modelName>"
"<modelNumber>%s</modelNumber>"
"<modelURL></modelURL>"
"<manufacturer>NTP</manufacturer>"
"<manufacturerURL></manufacturerURL>"
"<UDN>uuid:38323636-4558-4dda-9100-cda0e6%02x%02x%02x</UDN>"
"</device>"
"</root>\r\n"
"\r\n";
#endif /* _PSUFan_H_ */