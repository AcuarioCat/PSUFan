//WiFi settings
#pragma once
#ifndef _WiFiSettings_H_
#define _WiFiSettings_H_

const char * ssid = "Terrapico";
const char * password = "5432167890";
const char * hostName = "PSUFan";
const char * otaPassword = "j3ss1camPSU";

//Static IP address configuration
IPAddress staticIP(192, 168, 0, 102);	//ESP static ip
IPAddress gateway(192, 168, 0, 1);		//IP Address of WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);		//Subnet mask
IPAddress dns(8, 8, 8, 8);				//DNS

//Enable to set static ip address
//#define STATIC

#endif	//_WiFiSettings_H_