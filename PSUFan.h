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

extern const char * otaPassword;

#endif /* _PSUFan_H_ */