/**************************************************************************/
/*!
    @file     MixTape.ino
    @author   David Kadish
	@license  MIT (see license.txt)

    This script will write Spotify URIs to NFC tags and read them back, 
    triggering that song, album, artist, or playlist to play on your
    authenticated Spotify account.

*/
#include "defines.h"
#include "spotify.h"
#include "nfc.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Start the Serial connection
  Serial.begin(115200);
  while (!Serial) delay(10);

  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  nfcSetup();
  spotifySetup();

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  nfcLoop();
  spotifyLoop();
}
