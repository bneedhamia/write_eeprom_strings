/*
 * readWiFiConfig - example of reading configuration strings
 * from EEPROM.
 * 
 * Assumes the strings were written by write_eeprom_strings
 * into this Arduino's EEPROM.
 * 
 * Further assumes that
 *   the first EEPROM string is the WiFi SSID and
 *   the second EEPROM string is the WiFi Password.
 * 
 * Copyright (c) 2015 Bradford Needham
 * (@bneedhamia, https://www.needhamia.com)
 * Licensed under The MIT License (MIT)
 * a version of which should be supplied with this file.
 */
#include <EEPROM.h>

// First address of EEPROM to write to.
const int START_ADDRESS = 0;

// marks the end of the data we wrote to EEPROM
const byte EEPROM_END_MARK = 255;

// The WiFi SSID and Password, from EEPROM
char *wifiSsid;
char *wifiPassword;

void setup() {
  Serial.begin(9600);

  wifiSsid = readEEPROMString(START_ADDRESS, 0);
  wifiPassword = readEEPROMString(START_ADDRESS, 1);

  /*
   * Normally, you'd pass this SSID and Password to
   * the WiFi Shield initialization.
   * 
   * Since this is a simple demo, we'll just
   * print the strings.
   */

  if (wifiSsid == 0 || wifiPassword == 0) {
    Serial.println(F("EEPROM has not yet been initialized."));
    Serial.println(F("Please run write_eeprom_strings first."));
    return;
  }
  
  Serial.print(F("WiFi SSID = "));
  Serial.println(wifiSsid);

  Serial.print(F("Wifi Password = "));
  Serial.println(wifiPassword);

}

void loop() {
  // put your main code here, to run repeatedly:

}

/*
 * Reads a string from EEPROM.  Copy this code into your program that reads EEPROM.
 * 
 * baseAddress = EEPROM address of the first byte in EEPROM to read from.
 * stringNumber = index of the string to retrieve (string 0, string 1, etc.)
 * 
 * Assumes EEPROM contains a list of null-terminated strings,
 * terminated by EEPROM_END_MARK.
 * 
 * Returns:
 * A pointer to a dynamically-allocated string read from EEPROM,
 * or null if no such string was found.
 */
char *readEEPROMString(int baseAddress, int stringNumber) {
  int start;   // EEPROM address of the first byte of the string to return.
  int length;  // length (bytes) of the string to return, less the terminating null.
  char ch;
  int nextAddress;  // next address to read from EEPROM.
  char *result;     // points to the dynamically-allocated result to return.
  int i;

  nextAddress = START_ADDRESS;
  for (i = 0; i < stringNumber; ++i) {

    // If the first byte is an end mark, we've run out of strings too early.
    ch = (char) EEPROM.read(nextAddress++);
    if (ch == (char) EEPROM_END_MARK || nextAddress >= EEPROM.length()) {
      return (char *) 0;  // not enough strings are in EEPROM.
    }

    // Read through the string's terminating null (0).
    while (ch != '\0' && nextAddress < EEPROM.length()) {
      ch = EEPROM.read(nextAddress++);
    }
  }

  // We're now at the start of what should be our string.
  start = nextAddress;

  // If the first byte is an end mark, we've run out of strings too early.
  ch = (char) EEPROM.read(nextAddress++);
  if (ch == (char) EEPROM_END_MARK) {
    return (char *) 0;  // not enough strings are in EEPROM.
  }

  // Count to the end of this string.
  length = 0;
  while (ch != '\0' && nextAddress < EEPROM.length()) {
    ++length;
    ch = EEPROM.read(nextAddress++);
  }

  // Allocate space for the string, then copy it.
  result = new char[length + 1];
  nextAddress = start;
  for (i = 0; i < length; ++i) {
    result[i] = (char) EEPROM.read(nextAddress++);
  }
  result[i] = '\0';

  return result;

}

