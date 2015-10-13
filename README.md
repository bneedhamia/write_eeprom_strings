# write_eeprom_strings
An Arduino Sketch to write EEPROM with configuration strings such as Wifi SSID and Password.

Suppose you've written an Arduino Sketch that needs WiFi credentials, a user's name, an encryption key, or such. Write_eeprom_strings lets you put those configuration strings into EEPROM instead of hard-coding the strings into your Sketch.

Using write_eeprom_strings has the following advantages:

1. You can't accidentally commit code that has your passwords in it (admit it: you've done this).
2. Your configuration passwords aren't stored on your development computer.
3. You can set up a production line where one person enters passwords or other unique configuration into EEPROM and another person loads your Sketch into the Arduino.

## To Use

1. clone write_eeprom_strings.
2. Modify the STRING_NAME array to prompt for your configuration strings.  By default, write_eeprom_strings prompts for WiFi SSID and Password.
3. If you wish, modify START_ADDRESS to start writing to a different address in EEPROM.  By default, START_ADDRESS is 0.
4. Copy the readEEPROMString() function into your own Sketch, so that your Sketch can read the configuration strings that write_eeprom_strings writes to EEPROM.
5. Using the Arduino IDE, run your modified write_eeprom_strings Sketch.
6. Follow the prompts and type the configuration strings (such as your WiFi SSID and Password).
7. Load and run your own Sketch, which will read its configuration strings from EEPROM.
8. Repeat steps 5 through 7 for each Arduino you wish to program.
9. Enjoy the knowledge that your Sketch is now easily configurable!
