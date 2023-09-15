#ifndef PORTAL_H
#define PORTAL_H

#include <Arduino.h>

// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _WIFIMGR_LOGLEVEL_    4

#include <WiFi.h>

#include <WiFiMulti_Generic.h>
WiFiMulti_Generic wifiMulti;

#include <LittleFS.h>

#define  CONFIG_FILENAME        ("/wifi_cred.dat")

void check_status(void);
void portal_setup(void);

#endif