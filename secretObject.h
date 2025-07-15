/* Class for managing storing and retreving calibration data from persistant memory


Method developed by: David Reid
Author: Imogen Wren
14/07/2025


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
#include <EEPROM.h>



#define SECRETOBJECT_VERSION "V1.0.0"
#define EEPROM_START_ADDRESS 0xFF  // define start point for any persistant memory use
#define FLASH_WRITES_MAX 20        // define number of writes untill firmware must be re-programmed
#define SECRET_LEN_MAX 9

#define PROGRAM_SIGNATURE 0x98C7AB1E   //0x97B69A0D  // change this when re-writing firmware to enable overwrite of calibration data

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




  bool cal_is_secure();
  bool cal_is_valid();
  secretObject::calStruc get_cal();
  void cal_set_secret(const char *secret);
  void cal_set_values(int16_t calValue, const char *secret);

  void report_cal();


private:

  //EEprom Variables
  const int WRITTEN_SIGNATURE = PROGRAM_SIGNATURE;  //0x98C7AB1E;  // Arbitary signature to check for existing encoder offset value in persistant memory (practable)


  // Private cal struc -> stores the cal data in persistant memory, keep private to prevent library users accidently passing this structure out of the library and exposing the secret
  struct Calibration {
    int signature;                    // hard coded signature to avoid arbitary data being returned as true, written with secret
    bool secure;                      // Set this true when secret is first written
    bool valid;                       // Set this true when calibration data is first written
    char secret[SECRET_LEN_MAX + 1];  // Secret string for authorising calibration updates // now -> 8 char password (too long ->)(typically a uuid of 36 chars in form 8-4-4-4-12)
    int writes;                       // Count number of remaining writes we'll permit
    // You can change the values below here to suit your experiment
    int16_t data;  // Scale factors / calibration / offset data
  };

  // Create a global "Calibration" variable and call it cal
  Calibration cal;

  // Reserve a portion of flash memory to store a "Calibration" and
  // call it "cal_store".
  // dont think this works
  //FlashStorage_STM32(cal_store, Calibration);  // SAMD21 method only
};



#endif
