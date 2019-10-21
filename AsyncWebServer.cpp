// For project PSUFan
//
#include <ESP8266SSDP.h>
#include <StreamString.h>
#include <ArduinoJson.h>
#include "AsyncWebServer.h"

//SSDP properties
constexpr auto modelName = "PSU Fan control";
constexpr auto friendlyName = "PSU Fan control";
constexpr auto modelNumber = "";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocket wsd("/wsd");

extern bool manual;

/////////////////////////////////////  SSDP network discovery //////////////////////////////////////
void setupSSDP() {
	SSDP.setSchemaURL("description.xml");
	SSDP.setHTTPPort(80);
	SSDP.setDeviceType("upnp:rootdevice");
	SSDP.setModelName(modelName);
	SSDP.setModelNumber(modelNumber);
	SSDP.begin();

	server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest* request) {
		char hostName[25] = "PSU Fan control";
		char buf[7];

		StreamString output;
		if (output.reserve(1024)) {
			IPAddress ip = WiFi.localIP();
			uint32_t chipId = ESP.getChipId();
			output.printf(ssdpTemplate,
				ip[0], ip[1], ip[2], ip[3],
				hostName,
				ip[0], ip[1], ip[2], ip[3],
				chipId,
				modelName,
				modelNumber,
				(uint8_t)((chipId >> 16) & 0xff),
				(uint8_t)((chipId >> 8) & 0xff),
				(uint8_t)chipId & 0xff
			);
			request->send(200, "text/xml", (String)output);
		}
		else {
			request->send(500);
		}
		});
}

//Return a string with the system parameters as a json encapsulated in a function for processing on the browser
extern int fanSpeed;
extern int temperature;
extern bool fanState;
void sendSettings() {
	const size_t capacity = JSON_OBJECT_SIZE(20);
	DynamicJsonDocument root(capacity);

	// Add values in the object
	root["temp"] = temperature;
	root["fan"] = fanSpeed;
	root["fanbtn"] = fanState;

	String jsonString;
	serializeJson(root, jsonString);
	ws.textAll(jsonString);
}


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
	// Start and init debug winsocket
	ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
		{
			Serial.printf("wsd event: %d\n", type);
			if (type == WS_EVT_CONNECT)
			{
				//Serial.printf("wsd[%s][%u] connected\n", server->url(), client->id());
				client->ping();
			}
		});
	server.addHandler(&ws);

	// Start and init debug winsocket
	wsd.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
		{
			//Serial.printf("wsd event: %d\n", type);
			if (type == WS_EVT_CONNECT)
			{
				//Serial.printf("wsd[%s][%u] connected\n", server->url(), client->id());
				client->ping();
			}
		});
	server.addHandler(&wsd);

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

	server.on("/setup", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/setup.html");
		});

	server.on("/refresh", HTTP_GET, [](AsyncWebServerRequest* request) {
		sendSettings();
		request->send(200);
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
		manual = true;
		request->send(200, "text/html", "Fan on");
	});

	server.on("/fanoff", HTTP_POST, [](AsyncWebServerRequest *request) {

		DEBUG_PRINTLN("Fan off");
		manual = false;
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
	setupSSDP();
	server.begin();
	Serial.println("Server setup");
}

void sendWsMessage(String message, String key) {
	String jsonString;
	// Add values in the object
	const size_t capacity = JSON_OBJECT_SIZE(4);
	DynamicJsonDocument root(capacity);
	root[key] = message;
	serializeJson(root, jsonString);
	ws.textAll(jsonString);
}

void setupOTAUpdate(void)
{
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
