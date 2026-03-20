
/* Class for managing storing and retreving calibration data from persistant memory


Origional Method developed by: David Reid
Further Development: Imogen Wren
14/07/2025


*/

#include "secretObject.h"
#include <EEPROM.h>

secretObject::secretObject() {
}


bool secretObject::cal_is_secure() {
  // cal = cal_store.read();
  //Calibration cal_read;
  EEPROM.get(EEPROM_START_ADDRESS, cal);
  bool signature_matched = (cal.signature == WRITTEN_SIGNATURE);
  return (cal.secure && signature_matched);
}

bool secretObject::cal_is_valid() {
  // cal = cal_store.read();
  EEPROM.get(EEPROM_START_ADDRESS, cal);
  bool signature_matched = (cal.signature == WRITTEN_SIGNATURE);
  return (cal.secure && cal.calValid && signature_matched);
}

bool secretObject::settings_valid() {
  // cal = cal_store.read();
  EEPROM.get(EEPROM_START_ADDRESS, cal);
  bool signature_matched = (cal.signature == WRITTEN_SIGNATURE);
  return (cal.secure && secretObject::is_valid(&cal.settingsValid, 0b1111) && signature_matched);
}



secretObject::calStruc secretObject::get_cal() {
  calStruc memContents = { false, 0 };
  // cal = cal_store.read();
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  secretObject::report_cal();

  if (cal.signature != WRITTEN_SIGNATURE) {
    Serial.println(F("{\"WARNING\":\"get_cal signature did not match\"}"));
    return memContents;
  }
  if (cal.secure && cal.calValid) {
    memContents.calValid = true;
    memContents.calData = cal.calData;
    Serial.print(F("{\"INFO\":\"get_cal -> loaded\",\"cal\":\""));
    Serial.print(memContents.calData);
    Serial.println(F("\"}"));
  } else {
    if (!cal.secure) {
      Serial.println(F("{\"WARNING\":\"get_cal -> not secure\"}"));
    }
    if (!cal.calValid) {
      Serial.println(F("{\"WARNING\":\"get_cal -> not valid\"}"));
    }
  }
  return memContents;
}




//#TODO FIX THIS FUNCTION ->--ON WEDNESDAY---------------------------------------------------------------------------------------
secretObject::settingsStruc secretObject::get_settings() {
  // create empty structure for passing settings out of lib
  settingsStruc newSettings = { 0b0000, "", "", 0, 0 };
  // get the EEPROM contents
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  // Report mem contents to user?
  secretObject::report_cal();       // cal yes as this is used just for validation at this stage
  secretObject::report_settings();  // contents, no as this must be passed data after it has been checked for validity (because it might contain arrays that need correct termination)
                                    // this has now been fixed and this function should be safe to call here now

  // Check the written signature in mem matches written signature in firmware (validates that the data is current and not arbritary)
  if (cal.signature != WRITTEN_SIGNATURE) {
    Serial.println(F("{\"WARNING\":\"get_settings signature did not match\"}"));
    return newSettings;  // return empty structure if no valid signature
  }
  if (cal.secure && secretObject::is_valid(&cal.settingsValid, 0b1111)) {  // check that cal is secure i.e. a secret has been entered and that all 4 bits have been set for the settings
    newSettings.settingsValid = cal.settingsValid;
    secretObject::safe_copy_string(newSettings.material, cal.material, sizeof newSettings.material);  // get all the data from EEprom into newSettings
    secretObject::safe_copy_string(newSettings.diameter, cal.diameter, sizeof newSettings.diameter);
    newSettings.angleMax = cal.angleMax;
    newSettings.loadMax = cal.loadMax;

    Serial.print(F("{\"INFO\":\"get_settings -> loaded\""));
    Serial.println(F("\"}"));
    secretObject::report_settings();
  } else {
    if (!cal.secure) {
      Serial.println(F("{\"WARNING\":\"get_settings -> not secure\"}"));
    }
  }
  return newSettings;
}


void secretObject::set_secret(const char *secret) {
  if (secret[0] == '\0') {
    return;  //empty string
  }
  //cal = cal_store.read();
  EEPROM.get(EEPROM_START_ADDRESS, cal);
  if (cal.secure && cal.signature == WRITTEN_SIGNATURE) {  // only set secret once
    Serial.println(F("{\"ERROR\":\"secret already set\"}"));
  } else {
    strncpy(cal.secret, secret, (sizeof cal.secret) - 1);
    cal.secret[SECRET_LEN_MAX] = '\0';  // null terminator
    cal.secure = true;
    cal.writes = FLASH_WRITES_MAX;
    cal.signature = WRITTEN_SIGNATURE;
    cal.calValid = false;  // cal cannot be valid if just writing signature
    cal.settingsValid = 0b0000;
    EEPROM.put(EEPROM_START_ADDRESS, cal);
    // cal_store.write(cal);
    Serial.println(F("{\"INFO\":\"secret set\"}"));
  }
}



// NEW -> Check function
// modified so passed value instead of relying on user updating value then reading global
bool secretObject::cal_pre_check(secretObject::Calibration calStruc, const char *secret) {
  if (!calStruc.secure) {
    Serial.println(F("{\"ERROR\":\"cal_set -> cal secret not set\"}"));
    return 0;  // don't set values before setting authorisation (prevent rogue writes)
  }

  if (calStruc.writes <= 0) {
    Serial.println(F("{\"ERROR\":\"cal_set -> no more cal writes permitted - reflash firmware to reset counter\"}"));
    return 0;  // prevent writes if remaining write count has reached zero
  }

  if (!(strcmp(calStruc.secret, secret) == 0)) {
    Serial.println(F("{\"ERROR\":\"cal_set -> wrong secret\"}"));
    return 0;  // don't set values if auth code does not match secret
  }

  if (calStruc.signature != WRITTEN_SIGNATURE) {
    Serial.println(F("{\"ERROR\":\"cal_set -> written signature does not match\"}"));
    return 0;  // don't set values if auth code does not match secret auth code (set when first writing secret)
  }
  return true;
}




//## MODIFIED -> Check Function
void secretObject::set_cal_value(int16_t calValue, const char *secret) {
  //cal = cal_store.read();
  // get the Calibration structure from persistant memory
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  // do the precondition check
  if (!secretObject::cal_pre_check(cal, secret)) {
    return;  // if precheck fails, exit
  }


  // stuff for arrays, we will deal with this when we need to deal with it
  //   if (values.size() != SCALE_FACTOR_LEN) {
  //       Serial.print("{\"error\":\"wrong number of values in cal array\",");
  //       Serial.print("\"want\":");
  //       Serial.print(SCALE_FACTOR_LEN);
  //       Serial.print(",\"have\":");
  //       Serial.print(values.size());
  //      Serial.println("}");
  //      return; // don't set cal values if wrong number
  //      } //size ok
  //

  // Update all the values in the structure that need updating
  cal.writes -= 1;
  cal.calValid = true;
  cal.calData = calValue;

  // Print message to user with updated details
  Serial.print(F("{\"INFO\":\"cal_set -> cal ok\",\"value\":\""));
  Serial.print(cal.calData);
  Serial.print(F("\",\"writes_remaining\":"));
  Serial.print(cal.writes);
  Serial.println(F("}"));

  // store the data back into persistant memory
  //cal_store.write(cal);
  EEPROM.put(EEPROM_START_ADDRESS, cal);
}





void secretObject::set_settings_data(const char *material, const char *diameter, int16_t angleMax, int16_t loadMax, const char *secret) {
  // get the Calibration structure from persistant memory
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  // do the precondition check
  if (!secretObject::cal_pre_check(cal, secret)) {
    return;  // if precheck fails, exit
  }

  // Update all the values in the structure that need updating
  // First the utility datas
  cal.writes -= 1;
  cal.settingsValid = 0b1111;  // we are saving all datas in one so we do not need to increment

  // then the settings datas
  secretObject::safe_copy_string(cal.material, material, sizeof cal.material);  // copys string length of data structure and inserts null terminator
  secretObject::safe_copy_string(cal.diameter, diameter, sizeof cal.diameter);
  cal.angleMax = angleMax;
  cal.loadMax = loadMax;

  // Print message to user showing settings updated and writes remaining
  secretObject::settings_update_msg();
  // report the current (valid) settings
  secretObject::report_settings();


  // store the data back into persistant memory
  EEPROM.put(EEPROM_START_ADDRESS, cal);
}





void secretObject::set_material(const char *material, const char *secret) {  // These individal functions are going to be the only way to implement this if using JSONmessenger for now
                                                                             // get the Calibration structure from persistant memory
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  // do the precondition check
  if (!secretObject::cal_pre_check(cal, secret)) {
    return;  // if precheck fails, exit
  }

  // Update all the values in the structure that need updating
  // First the utility datas
  cal.writes -= 1;
  secretObject::set_bit(&cal.settingsValid, 0);  // set bit 0 as material has been updated


  // then the settings datas
  secretObject::safe_copy_string(cal.material, material, sizeof cal.material);  // copys string length of data structure and inserts null terminator
                                                                                // secretObject::safe_copy_string(cal.diameter, diameter, sizeof cal.diameter);
                                                                                //  cal.angleMax = angleMax;
                                                                                //  cal.loadMax = loadMax;

  // Print message to user showing settings updated and writes remaining
  secretObject::settings_update_msg();
  // report the current settings
  secretObject::report_settings();


  // store the data back into persistant memory
  EEPROM.put(EEPROM_START_ADDRESS, cal);
}



void secretObject::set_diameter(const char *diameter, const char *secret) {
  // get the Calibration structure from persistant memory
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  // do the precondition check
  if (!secretObject::cal_pre_check(cal, secret)) {
    return;  // if precheck fails, exit
  }

  // Update all the values in the structure that need updating
  // First the utility datas
  cal.writes -= 1;
  secretObject::set_bit(&cal.settingsValid, 1);  // set bit 1 as diamter has been updated

  // then the settings datas
  // secretObject::safe_copy_string(cal.material, material, sizeof cal.material);  // copys string length of data structure and inserts null terminator
  secretObject::safe_copy_string(cal.diameter, diameter, sizeof cal.diameter);
  //  cal.angleMax = angleMax;
  //  cal.loadMax = loadMax;

  // Print message to user showing settings updated and writes remaining
  secretObject::settings_update_msg();
  // report the current settings
  secretObject::report_settings();

  // store the data back into persistant memory
  EEPROM.put(EEPROM_START_ADDRESS, cal);
}



void secretObject::set_angle_max(int16_t angleMax, const char *secret) {
  // get the Calibration structure from persistant memory
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  // do the precondition check
  if (!secretObject::cal_pre_check(cal, secret)) {
    return;  // if precheck fails, exit
  }

  // Update all the values in the structure that need updating
  // First the utility datas
  cal.writes -= 1;
  secretObject::set_bit(&cal.settingsValid, 2);  // set bit 2 as anglemax has been updated

  // then the settings datas
  // secretObject::safe_copy_string(cal.material, material, sizeof cal.material);  // copys string length of data structure and inserts null terminator
  // secretObject::safe_copy_string(cal.diameter, diameter, sizeof cal.diameter);
  cal.angleMax = angleMax;
  //  cal.loadMax = loadMax;

  // Print message to user showing settings updated and writes remaining
  secretObject::settings_update_msg();
  // report the current settings
  secretObject::report_settings();

  // store the data back into persistant memory
  EEPROM.put(EEPROM_START_ADDRESS, cal);
}



void secretObject::set_load_max(int16_t loadMax, const char *secret) {
  // get the Calibration structure from persistant memory
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  // do the precondition check
  if (!secretObject::cal_pre_check(cal, secret)) {
    return;  // if precheck fails, exit
  }

  // Update all the values in the structure that need updating
  // First the utility datas
  cal.writes -= 1;
  secretObject::set_bit(&cal.settingsValid, 3);  // set bit 3 as loadmax has been updated

  // then the settings datas
  // secretObject::safe_copy_string(cal.material, material, sizeof cal.material);  // copys string length of data structure and inserts null terminator
  // secretObject::safe_copy_string(cal.diameter, diameter, sizeof cal.diameter);
  //  cal.angleMax = angleMax;
  cal.loadMax = loadMax;

  // Print message to user showing settings updated and writes remaining
  secretObject::settings_update_msg();
  // report the current settings
  secretObject::report_settings();


  // store the data back into persistant memory
  EEPROM.put(EEPROM_START_ADDRESS, cal);
}



// This does report globals after being retreved directly from memory. This is okay, as it does not contain arrays so it may contain junk data but none of this should have an impact on function
void secretObject::report_cal() {
  Serial.print(F("{\"INFO\":\"cal_report\",\"signature\":\""));
  Serial.print(cal.signature);
  Serial.print(F("\",\"secure\":\""));
  Serial.print(cal.secure);
  Serial.print(F("\",\"valid\":\""));
  Serial.print(cal.calValid);
  Serial.print(F("\",\"writes_left\":\""));
  Serial.print(cal.writes);
  Serial.print(F("\",\"cal-data\":\""));
  Serial.print(cal.calData);
  Serial.println(F("\"}"));
}


// This shouldnt print the global vars it should be passed settingsStruct and only be passed valid data to avoid recalling junk & unterminated arrays from arbritatry memory
// this wont avoid people trying to pass it junk, but it will error if passed the struct direct from memory, providing a warning to users who modify this code for their own purposed
// NO NO NO, as high falutin as this idea was it actually causes way more issues than it solves, and is causing code bloat.
// Instead it will just report the global vars but USER MUST MAKE SURE DATA HAS ALREADY BEEN CHECKED FOR VALIDITY
// only nope, this still isnt good enough because the calling functions will call it before that data has been entered, so instead it has to check itself if the data is valid
// oh wait, we already wrote a function to do that and we have the var that tells us whether its valid right there
void secretObject::report_settings() {
  Serial.print(F("{\"INFO\":\"report_settings\""));
  Serial.print(F(",\"valid\":\""));
  secretObject::print_bin(cal.settingsValid);
  if (is_valid(&cal.settingsValid, 0b0001)) {
    Serial.print(F("\",\"material\":\""));
    Serial.print(cal.material);
  }
  if (is_valid(&cal.settingsValid, 0b0010)) {
    Serial.print(F("\",\"diameter\":\""));
    Serial.print(cal.diameter);
  }
  if (is_valid(&cal.settingsValid, 0b0100)) {
    Serial.print(F("\",\"angle_max\":\""));
    Serial.print(cal.angleMax);
  }
  if (is_valid(&cal.settingsValid, 0b1000)) {
    Serial.print(F("\",\"load_max\":\""));
    Serial.print(cal.loadMax);
  }
  Serial.println(F("\"}"));
}


// Call this in this format secretObject::safe_copy_string(target, newString , sizeof target);
void secretObject::safe_copy_string(char *target, const char *newString, uint8_t targetLength) {
  strncpy(target, newString, targetLength - 1);
  target[targetLength - 1] = '\0';  // null terminator
}


void secretObject::set_bit(uint8_t *valid, uint8_t bit) {  // used for flagging using single bits
  *valid |= (0b0001 << bit);
}


bool secretObject::is_valid(uint8_t *valid, const uint8_t key) {  // compares the flag variable against a key value
  return ((*valid & key) == key);
}



void secretObject::print_bin(uint8_t binVal) {
  char buffer[9];
  for (int i = 4; i < 8; i++) {  // to get the first 4 bits need to start at the thrd
    sprintf(buffer, "%d", !!((binVal << i) & 0x80));
    Serial.print(buffer);
  }
  //  Serial.print("\n");  doesnt need a newline
}

void secretObject::settings_update_msg() {
  // Print message to user showing settings updated and writes remaining
  Serial.print(F("{\"INFO\":\"settings_updated\""));
  Serial.print(F("\",\"writes_remaining\":"));
  Serial.print(cal.writes);
  Serial.println(F("}"));
}
