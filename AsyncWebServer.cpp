// For project PSUFan
//
#include "AsyncWebServer.h"

AsyncWebServer server(8081);
AsyncEventSource debugEvent("/debugMsg"); // event source (Server-Sent events)
AsyncEventSource events("/updates"); // event source (Server-Sent events)

AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws

void onRequest(AsyncWebServerRequest *request) {
	//Handle Unknown Request
	request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
	//Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	//Handle upload
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
	//Handle WebSocket event
}

void setupWebserver() {
	// attach AsyncWebSocket
	ws.onEvent(onEvent);
	server.addHandler(&ws);

	// attach AsyncEventSource
	server.addHandler(&events);

	server.on("/devReboot", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", "Rebooting...");
		shouldReboot = true;
	});

	// Reset all system parameters to default
	server.on("/resetParam", HTTP_GET, [](AsyncWebServerRequest *request) {
		resetParameters();
		request->send(200, "text/plain", "All parameters reset to default");
	});

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/status.html");
	});

	server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/status.html");
	});

	server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/setup.html");
	});

	server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
		DEBUG_PRINTLN("settings");

		String strHTML = buildSettingsXML();
		request->send(200, "text/xml", strHTML);
	});

	server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
		DEBUG_PRINTLN("save");
		if (request->args() > 0)
		{
			fanOnTemp = (int)request->arg("i").toInt();
			fanOffTemp = (int)request->arg("c").toInt();
			saveSettings();
		}
		request->send(200, "text/plain", "Saved settings");
	});

	server.on("/fanon", HTTP_POST, [](AsyncWebServerRequest *request) {

		DEBUG_PRINTLN("Fan on");
		turnFan(1023);
		request->send(200, "text/html", "Fan on");
	});

	server.on("/fanoff", HTTP_POST, [](AsyncWebServerRequest *request) {

		DEBUG_PRINTLN("Fan off");
		turnFan(0);
		request->send(200, "text/html", "Fan off");
	});

	//Serve all files. 
	server.serveStatic("/", SPIFFS, "/", "max-age=2592000");

	// Catch-All Handlers
	// Any request that can not find a Handler that canHandle it ends in the callbacks below.
	server.onNotFound(onRequest);
	server.onFileUpload(onUpload);
	server.onRequestBody(onBody);

	server.begin();
}

void setupOTAUpdate(void)
{
	// Hostname defaults to esp8266-[ChipID]
	// ArduinoOTA.setHostname("myesp8266");

	// Authentication password
	ArduinoOTA.setPassword(otaPassword);

	ArduinoOTA.onStart([]() {
		DEBUG_PRINTLN("Start");
	});
	ArduinoOTA.onEnd([]() {
		DEBUG_PRINTLN("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
		else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
	});
	ArduinoOTA.begin();
}
