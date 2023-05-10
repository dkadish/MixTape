/****************************************************************************************************************************
  defines.h
  
  A bare-bones version of the defines.h from
  https://github.com/witnessmenow/spotify-api-arduino/blob/main/examples/getRefreshToken/getRefreshToken.ino
  with definintions for the MKR WiFi 1010
  
  Licensed under MIT license
 ***************************************************************************************************************************************/

#ifndef defines_h
#define defines_h

#include "secrets.h"

#define DEBUG_WIFI_WEBSERVER_PORT   Serial

// Debug Level from 0 to 4
#define _WIFI_LOGLEVEL_             4
#define _WIFININA_LOGLEVEL_         3

#define USE_WIFI_NINA         true
#define USE_WIFI101           false
#define USE_WIFI_CUSTOM       false

#warning Using WiFiNINA using WiFiNINA_Generic Library
#define SHIELD_TYPE           "WiFiNINA using WiFiNINA_Generic Library"

#if defined(WIFI_USE_SAMD)
  #undef WIFI_USE_SAMD
#endif
#define WIFI_USE_SAMD      true

#define BOARD_TYPE      "SAMD MKRWIFI1010"

#include <WiFiWebServer.h>

char ssid[] = SECRET_SSID;         // your network SSID (name)
char password[] = SECRET_PASS; // your network password

char clientId[] = SECRET_CLIENT_ID;     // Your client ID of your spotify APP
char clientSecret[] = SECRET_CLIENT_SECRET; // Your client Secret of your spotify APP (Do Not share this!)

#define USE_IP_ADDRESS 1 

// Country code, including this is advisable
#define SPOTIFY_MARKET "SE"

#endif    //defines_h