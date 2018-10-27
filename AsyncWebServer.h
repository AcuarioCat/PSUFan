#pragma once
#ifndef _AsyncWebServer_H_
#define _AsyncWebServer_H_

#include <ArduinoOTA.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "PSUFan.h"

void setupWebserver();
void setupOTAUpdate(void);

#endif //_AsyncWebServer_H_