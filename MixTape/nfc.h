/**************************************************************************/
/*!
    @file     MixTape.ino
    @author   David Kadish
	@license  MIT (see license.txt)

    Functions to deal with authenticating, reading, and writing NFC tags.

*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// Define the pins for SPI communication with the PN532
#define PN532_SCK  (9)
#define PN532_MOSI (8)
#define PN532_SS   (3)
#define PN532_MISO (10)

// Create the PN532 object using an SPI connection
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Boolean variable to track the presence of a tag to avoid multiple reads
bool nfcPresent = false;

/* Authenticate an NFC tag and return whether authentication has been successful.
 *
 * param blocking: [bool] Should the function block execution until a tag has been found or should it run with a timeout to allow other code to execute.
 * 
 * return: true, if NFC tag has been successfuly authenticated, otherwise false.
 */
bool authNFC(bool blocking){
  // Variables to store the results of a card read
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  // Set variables to block or use a timeout
  int timeout = 100;
  if(blocking){
    nfcPresent = false;
    timeout = 0;
  }
  
  // Read a card
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
  
  // Update card presence and return false if necessary
  if(!success){ // Unsuccessful read. Return false and reset presence.
    nfcPresent = false;
    return false;
  } else if( nfcPresent && success ){ // Successful read, but for already read card.
    return false;
  }

  // Set presence to true
  nfcPresent = true;

  // Display some basic information about the card
  Serial.println("Found an ISO14443A card");
  Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
  Serial.print("  UID Value: ");
  nfc.PrintHex(uid, uidLength);
  Serial.println("");

  if (uidLength == 4) // Only use Mifare Classic cards. Untested on Mifare Ultralite
  {
    // Now we need to try to authenticate it for read/write access
    // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
    Serial.println("Trying to authenticate block 4 with default KEYA value");
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    // Start with block 4 (the first block of sector 1) since sector 0
    // contains the manufacturer data and it's probably better just
    // to leave it alone unless you know what you're doing
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);

    if (success)
      return true;
  } else {
    Serial.println("Seems like you have the wrong type of card for this example. Please try a Mifare Classic card (4 byte UID)");
  }

  return false;
}

/* Write a Spotify URI to an NFC tag.
 *
 * param uri: [String] String containing a spotify URI  (e.g. "spotify:track:6rqhFgbbKwnb9MLmUQDhG6") to write to the tag
 */
void writeSpotifyCode(String uri){
  if(authNFC(true)){
    Serial.println("Sector 1 (Blocks 4..7) has been authenticated");
    uint8_t data[16];

    // Break the uri up into 16-byte arrays (max 48 bytes) and write to sectors 4-6.
    for(int i=0; i < 3; i++){
      if(uri.length() > 16){ // If the URI has more than 16 chars left, write the first 16
        memcpy(data, uri.c_str(), 16);
        uri = uri.substring(16);
      } else if (uri.length() == 16) { // If the URI has exactly 16 chars, write them and leave a newline char
        memcpy(data, uri.c_str(), 16);
        uri = "\n";
      } else { // If the URI has less than 16 chars left, add a newline, then fill with null characters and write
        uri += "\n";
        memcpy(data, uri.c_str(), uri.length());
        for(int j=uri.length(); j < 16; j++) data[j] = '\0';
        uri = "";
      }

      //TODO: Do something with success variable.
      uint8_t success = nfc.mifareclassic_WriteDataBlock (4+i, data);

      if(uri.length() == 0){
        break;
      }
    }
    Serial.println("Blocks Written");
  }
}

/* Read a Spotify code from an NFC tag
 * 
 * return: [String] String object containing the Spotify URI read from the tag.
 */
String readSpotifyCode(){
  String track = "";
  if(authNFC(false)){
    uint8_t data[16];

    for(int i=0; i < 3; i++){
      // Try to read the contents of block 4+i
      //TODO: Do something with success variable.
      uint8_t success = nfc.mifareclassic_ReadDataBlock(4+i, data);

      if (success)
      {
        // Data seems to have been read ... spit it out
        Serial.print("Reading Block ");
        Serial.print(4+i);
        Serial.println(":");
        nfc.PrintHexChar(data, 16);
        Serial.println("");

        track += (char *)data;
        track.trim(); // Get rid of some trailing characters
        // Check if last character should be removed
        if(track.charAt(track.length()-1) > 127){

          track = track.substring(0,track.length()-1);
        }
      }
      else
      {
        Serial.println("Ooops ... unable to read the requested block.  Try another key?");
      }
    }

    int newline = track.indexOf('\n');
    if(newline > 0){
      track = track.substring(0, newline);
    }

    Serial.print("Track to play: ");
    Serial.println(track);
  }
  return track;
}

void nfcSetup(){
  nfc.begin();

  // Check connection to the NFC board.
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
}

void nfcLoop(){
  if (Serial.available() == 0){
    String uri = readSpotifyCode();
    if(uri.length() > 0){
      play(uri);
    }
  } else {
    String spotifyURI = Serial.readString();// Should be a spotify URI e.g. "spotify:track:3cRmHMdcalueasgmweHd2r"
    spotifyURI.trim();
    Serial.println(spotifyURI);
    writeSpotifyCode(spotifyURI);
  }
}
