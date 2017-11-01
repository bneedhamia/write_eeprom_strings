/*
 * Writes a set of user-input strings into EEPROM.
 * 
 * This Sketch writes the following data to EEPROM,
 * starting at START_ADDRESS:
 * 
 * Null-terminated string0 (e.g., WiFi SSID)
 * Null-terminated string1 (e.g., WiFi Password)
 * ...
 * EEPROM_END_MARK byte (see below)
 * 
 * To change what the Sketch asks for user input,
 * change the STRING_NAME array.
 * For example, you could add a string after "WiFi Password"
 * to ask the user for, for example, "Encryption Type (wep, wep2, none)".
 * 
 * To change what address the Sketch writes in EEPROM,
 * change START_ADDRESS.
 * 
 * Copyright (c) 2015, 2016, 2017 Bradford Needham
 * @bneedhamia, https://www.needhamia.com
 * Licensed under The MIT License (MIT)
 * a version of which should be supplied with this file.
 */
#include <EEPROM.h>

// First address of EEPROM to write to.
const int START_ADDRESS = 0;

// The names of the strings to put into EEPROM, in order.  Null ends the strings.
const char *STRING_NAME[] = {
  "SSID",
  "Password",
  "Timeout(seconds)",
  0
};

// marks the end of the data we wrote to EEPROM
const byte EEPROM_END_MARK = 255;

// Maximum bytes (including null) of any string we support.
const int EEPROM_MAX_STRING_LENGTH = 120;

// The state machine that drives the loop.
enum state {
  PROMPT,             // prompt the user with the STRING_NAME[]
  WAIT_FOR_STRING,    // wait for keyboard input
  WAIT_FOR_YN,        // wait for a "y" or "n" from the user (Yes or No)
  READ_BACK,          // report the contents written to the EEPROM
  DONE                // finished writing all strings to EEPROM
};
byte currentState;

// Address of the next byte in EEPROM to write.
int nextEEPROMaddress;

// Number of strings we have written to EEPROM so far.
int numberOfStrings;

// The buffer that holds the string being read from the user.
char buf[EEPROM_MAX_STRING_LENGTH];
int nextIndex;              // index of the next empty space in buf[] to fill.

// if null, we're waiting for the y or n. Otherwise, we're waiting for the end of line.
char yOrN;

void setup() {
  Serial.begin(9600);
  delay(200);  // give the uart time to power up
  Serial.println(F("RESET"));

  numberOfStrings = 0;
  nextEEPROMaddress = START_ADDRESS;
  currentState = PROMPT;

  /*
   * Don't change the EEPROM yet because we may have been reset accidentally,
   * for example, as the programmer is trying to load the program that uses the EEPROM we wrote.
   */

}

void loop() {
  char ch = 0;
  
  switch (currentState) {
  case PROMPT:
    Serial.print(STRING_NAME[numberOfStrings]);
    Serial.print(F(": "));

    // Set up to read the first string from the keyboard.
    
    nextIndex = 0;
    currentState = WAIT_FOR_STRING;
    break;
    
  case WAIT_FOR_STRING:
    if (Serial.available() == 0) {
      return; // nothing to do yet.
    }
    buf[nextIndex] = Serial.read();

    if (buf[nextIndex] != '\n' && nextIndex < EEPROM_MAX_STRING_LENGTH - 1) {
      ++nextIndex;
      return;
    }
    // End of string.
    buf[nextIndex] = '\0';
    
    Serial.println(buf);
    Serial.print(STRING_NAME[numberOfStrings]);
    Serial.print(F("="));
    Serial.print(buf);
    Serial.print(F("? (y or n): "));

    yOrN = 0;
    currentState = WAIT_FOR_YN;

    break;

  case WAIT_FOR_YN:
    if (Serial.available() == 0) {
      return;
    }
    ch = Serial.read();
    if (yOrN == 0) {
      yOrN = ch;
    }
    
    // Throw away characters until we read the end of the line
    if (ch != '\n') {
      return;
    }
    Serial.println(yOrN);

    // we've received the end of the line.
    // Check what the first character of this line was.


    if (yOrN != 'y' && yOrN != 'n') {     
      Serial.print(F("Please type a y or n"));
      Serial.println();
      Serial.print(STRING_NAME[numberOfStrings]);
      Serial.print(F("="));
      Serial.print(buf);
      Serial.print(F("? (y or n): "));

      yOrN = 0;
      return;
    }
    if (yOrN != 'y') {
      // The user didn't like what they typed.
      // Let the user re-type the input.
      currentState = PROMPT;
      return;
    }

    // The user said "yes".  Write the string to EEPROM
    if (!addToEEPROM(buf)) {
      Serial.println(F("Write failed.  Reset to try again."));
      currentState = DONE;
      return;
    }

    // Get ready to read the next string (or we're done)
    numberOfStrings++;
    if (STRING_NAME[numberOfStrings] == (char *) 0) {

      // Write an "end of strings" marker
      EEPROM.write(nextEEPROMaddress++, EEPROM_END_MARK);
      
      Serial.print(F("Used "));
      Serial.print(nextEEPROMaddress - START_ADDRESS);
      Serial.println(F(" bytes of EEPROM."));

      currentState = READ_BACK;
      return;
    }
    currentState = PROMPT;
    break;

  case READ_BACK:
    readEEPROM();
    currentState = DONE;
    break;

  case DONE:
    // nothing to do.
    break;
    
  }

}

/*
 * Append the given null-terminated string to the EEPROM.
 * Return true if successful; false otherwise.
 */
boolean addToEEPROM(char *text) {
  // Unfortunately, we can't know how much EEPROM is left.

#if defined(ESP8266) || defined(ESP32)
  /*
   * The ESP8266 and ESP32 EEPROM library differs from
   * the standard Arduino library. It is a cached model,
   * I assume to minimize limited EEPROM write cycles.
   */
  EEPROM.begin(512);
#endif

  do {
    EEPROM.write(nextEEPROMaddress++, (byte) *text);
  } while (*text++ != '\0');

#if defined(ESP8266) || defined(ESP32)
  EEPROM.end();
#endif

  Serial.println(F("Written to EEPROM."));

  return true;
}

/*
 * Read back and report the list of strings in EEPROM.
 * This function is an example of how to read EEPROM strings
 * that our Sketch wrote.
 */
void readEEPROM() {
  int i;
  char *s;
  
  Serial.println();
  Serial.print(F("EEPROM Contents starting at address "));
  Serial.println(START_ADDRESS);

  i = 0;
  while (true) {
    s = readEEPROMString(START_ADDRESS, i++);
    if (s == (char *) 0) {
      Serial.println(F("--END--"));
      break;
    }
    Serial.println(s);
    delete s;
    s = (char *) 0;
  }
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


#if defined(ESP8266) || defined(ESP32)
  EEPROM.begin(512);
#endif

  nextAddress = START_ADDRESS;
  for (i = 0; i < stringNumber; ++i) {

    // If the first byte is an end mark, we've run out of strings too early.
    ch = (char) EEPROM.read(nextAddress++);
    if (ch == (char) EEPROM_END_MARK) {
#if defined(ESP8266) || defined(ESP32)
      EEPROM.end();
#endif

      return (char *) 0;  // not enough strings are in EEPROM.
    }

    // Read through the string's terminating null (0).
    int length = 0;
    while (ch != '\0' && length < EEPROM_MAX_STRING_LENGTH - 1) {
      ++length;
      ch = EEPROM.read(nextAddress++);
    }
  }

  // We're now at the start of what should be our string.
  start = nextAddress;

  // If the first byte is an end mark, we've run out of strings too early.
  ch = (char) EEPROM.read(nextAddress++);
  if (ch == (char) EEPROM_END_MARK) {
#if defined(ESP8266) || defined(ESP32)
    EEPROM.end();
#endif
    return (char *) 0;  // not enough strings are in EEPROM.
  }

  // Count to the end of this string.
  length = 0;
  while (ch != '\0' && length < EEPROM_MAX_STRING_LENGTH - 1) {
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

