/* Class for managing storing and retreving settings & calibration data from persistant memory


Method developed by: David Reid
Author: Imogen Wren
14/07/2025

Modifications: 25/11/2025
Adding settings structure that allows for different datatypes to be stored
and recalled via an additional structure


*/


//#pragma once

#ifndef secretObject_h
#define secretObject_h

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//#include <FlashStorage_STM32.h>
//#include <EEPROM.h>  -> moved to .cpp file to stop warnings



#define SECRETOBJECT_VERSION "V1.2.0"  // backwards compatable with 1.0.0
#define EEPROM_START_ADDRESS 0x10     // define start point for any persistant memory use
#define FLASH_WRITES_MAX 25            // define number of writes untill firmware must be re-programmed
#define SECRET_LEN_MAX 9

#define PROGRAM_SIGNATURE 0x00AAF  // // change this when re-writing firmware to enable overwrite of calibration data

//#define MEMORY_OBJECT FlashStorage_STM32  // might actually just be EEPROM  // replace with suitable persistant memory object for platform
// Arduino SAMD21: FlashStorage    // untested
// Arduino ARM: EEPROM


class secretObject {

public:

  // constructor
  secretObject();


  // Public Calibration struct -> modify for specific use case (needed to pass cal data + flag out of library)
  struct calStruc {
    bool calValid;
    int16_t calData;
  };

  // keeping these two structures seperate as changing one does not always neccessitate changing the other

  // Public settings struct -> modify for specific use case (needed to pass settings data + flag out of library)
  struct settingsStruc {
    uint8_t settingsValid;  // instead of bool this should be int which sets bits true for each data that is entered
    char material[16];
    char diameter[8];
    int16_t angleMax;
    int16_t loadMax;
  };

  // due to current limitiations in jsonMessenger, each settings value will need its own state and function call
  // jsonMessenger has to change to accomidate materials char string with auth code



  bool cal_is_secure();
  bool cal_is_valid();
  bool settings_valid();
  secretObject::calStruc get_cal();
  secretObject::settingsStruc get_settings();
  void set_secret(const char *secret);
  void set_cal_value(int16_t calValue, const char *secret);
  void set_settings_data(const char *material, const char *diameter, int16_t angleMax, int16_t loadMax, const char *secret);  // this would be a fantastic way of building this functionality, however due to limitations in jsonMessenger, its not going to work
  void set_material(const char *material, const char *secret);                                                                // These individal functions are going to be the only way to implement this for now
  void set_diameter(const char *diameter, const char *secret);
  void set_angle_max(int16_t angleMax, const char *secret);
  void set_load_max(int16_t loadMax, const char *secret);
  void report_cal();
  void report_settings();
  void safe_copy_string(char *target,const char *newString, uint8_t targetLength);


private:




  //EEprom Variables
  const int WRITTEN_SIGNATURE = PROGRAM_SIGNATURE;  //0x98C7AB1E;  // Arbitary signature to check for existing encoder offset value in persistant memory (practable)


  // Private cal struc -> stores the cal data in persistant memory, keep private to prevent library users accidently passing this structure out of the library and exposing the secret
  struct Calibration {
    int signature;  // hard coded signature to avoid arbitary data being returned as true, written with secret
    bool secure;    // Set this true when secret is first written
    bool calValid;  // Set this true when calibration data is first written -> want to change this to calValid, but preserving backwards compatability
    uint8_t settingsValid;
    char secret[SECRET_LEN_MAX + 1];  // Secret string for authorising calibration updates // now -> 8 char password (too long ->)(typically a uuid of 36 chars in form 8-4-4-4-12)
    int writes;                       // Count number of remaining writes we'll permit
                                      // You can change the values below here to suit your experiment
    char material[16];
    char diameter[8];
    int16_t angleMax;
    int16_t loadMax;
    int16_t calData;  // Scale factors / calibration / offset data
  };

  // Create a global "Calibration" variable and call it cal
  Calibration cal;

  // Reserve a portion of flash memory to store a "Calibration" and
  // call it "cal_store".
  // dont think this works
  //FlashStorage_STM32(cal_store, Calibration);  // SAMD21 method only

  // Private Functions

  bool cal_pre_check(secretObject::Calibration calStruc, const char *secret);
  void set_bit(uint8_t *valid, uint8_t bit);         // used for flagging using single bits
  bool is_valid(uint8_t *valid, const uint8_t key);  // compares the flag variable against a key value
  void print_bin(uint8_t binVal);                    // prints a 4 bit int as binary
  void settings_update_msg();                        // updates user that settings have been updated
};



#endif
