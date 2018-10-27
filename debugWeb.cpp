#include "debugWeb.h"

void DebugWeb::sendWeb(const char * data)
{
	char* debugString;
	debugString = (char *)malloc(strlen(data) + 30);

	sprintf(debugString, "[%s][%d] %s", ntpTime.c_str(), ESP.getFreeHeap(), data);
	Serial.print(debugString);
	if(wifiActive){
		debugEvent.send(debugString, "debug");
	}
	free(debugString);
}

void DebugWeb::sendWebln(const char * data)
{
	char* debugString;
	debugString = (char *)malloc(strlen(data) + 30);

	sprintf(debugString, "[%s][%d] %s", ntpTime.c_str(), ESP.getFreeHeap(), data);
	Serial.println(debugString);
	if (wifiActive) {
		debugEvent.send(debugString, "debug");
	}
	free(debugString);
}
