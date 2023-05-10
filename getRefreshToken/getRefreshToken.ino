/*******************************************************************
    Get Refresh Token from spotify, this is needed for the main MixTape sketch.

    Instructions:

    - Put in your Wifi details, Client ID, Client secret and flash to the board
    - Get the Ip Address from the serial monitor
    - Add the following to Redirect URI on your Spotify app "http://[IP]/callback/"
    e.g. "http://192.168.1.20/callback/" (don't forget the last "/")
    - Open browser to the IP 
    - Click the link on the webpage
    - The Refresh Token will be printed to screen, use this
      for SPOTIFY_REFRESH_TOKEN in secrets.h.

    Compatible Boards:
	  - MKR WiFi 1010

    Attributions:
    - Based on the getRefreshToken sketch in Brian Lough's excellent spotify-api-arduino library
    - https://github.com/witnessmenow/spotify-api-arduino
 *******************************************************************/
#define SPOTIFY_DEBUG

#include "defines.h"

// Library for connecting to the Spotify API (install from zip)
// https://github.com/witnessmenow/spotify-api-arduino
#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>

// Library used for parsing Json from the API responses (install using Library Manager)
// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson.h>

char scope[] = "user-read-playback-state%20user-modify-playback-state";

String callbackURI;

WiFiWebServer server(80);

// WiFiClientSecure client;
WiFiSSLClient client;
SpotifyArduino spotify(client, clientId, clientSecret);

const char *webpageTemplate =
    R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
  </head>
  <body>
    <div>
     <a href="https://accounts.spotify.com/authorize?client_id=%s&response_type=code&redirect_uri=%s&scope=%s">spotify Auth</a>
    </div>
  </body>
</html>
)";

void handleRoot()
{
  char webpage[800];
  sprintf(webpage, webpageTemplate, clientId, callbackURI.c_str(), scope);
  server.send(200, "text/html", webpage);
  Serial.println(webpage);
}

void handleCallback()
{
  String code = "";
  const char *refreshToken = NULL;
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "code")
    {
      code = server.arg(i);
      refreshToken = spotify.requestAccessTokens(code.c_str(), callbackURI.c_str());
    }
  }

  if (refreshToken != NULL)
  {
    server.send(200, "text/plain", refreshToken);
  }
  else
  {
    server.send(404, "text/plain", "Failed to load token, check serial monitor");
  }
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  Serial.print(message);
  server.send(404, "text/plain", message);
}

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(10);

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  IPAddress ipAddress = WiFi.localIP();
  Serial.println(ipAddress);

  callbackURI = String("http://") + ipAddress[0] + "." + ipAddress[1] + "." + ipAddress[2] + "." + ipAddress[3] + String("/callback/");

  server.on("/", handleRoot);
  server.on("/callback/", handleCallback);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
}