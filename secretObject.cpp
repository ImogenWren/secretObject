
/* Class for managing storing and retreving calibration data from persistant memory


Origional Method developed by: David Reid
Further Development: Imogen Wren
14/07/2025


*/

#include "secretObject.h"


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
  return (cal.secure && cal.valid && signature_matched);
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
  if (cal.secure && cal.valid) {
    memContents.calValid = true;
    memContents.calData = cal.data;
    Serial.print(F("{\"INFO\":\"get_cal -> loaded\",\"cal\":\""));
    Serial.print(memContents.calData);
    Serial.println(F("\"}"));
  } else {
    if (!cal.secure) {
      Serial.println(F("{\"WARNING\":\"get_cal -> not secure\"}"));
    }
    if (!cal.valid) {
      Serial.println(F("{\"WARNING\":\"get_cal -> not valid\"}"));
    }
  }
  return memContents;
}



void secretObject::cal_set_secret(const char *secret) {
  if (secret[0] == '\0') {
    return;  //empty string
  }

  //cal = cal_store.read();
  EEPROM.get(EEPROM_START_ADDRESS, cal);


  if (cal.secure && cal.signature == WRITTEN_SIGNATURE) {  // only set secret once
    Serial.println(F("{\"ERROR\":\"secret already set\"}"));
  } else {
    strncpy(cal.secret, secret, (sizeof cal.secret) - 1);
    cal.secret[SECRET_LEN_MAX] = '\0';   // null terminator
    cal.secure = true;
    cal.writes = FLASH_WRITES_MAX;
    cal.signature = WRITTEN_SIGNATURE;
    cal.valid = false;  // cal cannot be valid if just writing signature
    EEPROM.put(EEPROM_START_ADDRESS, cal);
    // cal_store.write(cal);
    Serial.println(F("{\"INFO\":\"secret set\"}"));
  }
}


void secretObject::cal_set_values(int16_t calValue, const char *secret) {
  //cal = cal_store.read();
  EEPROM.get(EEPROM_START_ADDRESS, cal);

  if (!cal.secure) {
    Serial.println(F("{\"ERROR\":\"cal_set -> cal secret not set\"}"));
    return;  // don't set values before setting authorisation (prevent rogue writes)
  }

  if (cal.writes <= 0) {
    Serial.println(F("{\"ERROR\":\"cal_set -> no more cal writes permitted - reflash firmware to reset counter\"}"));
    return;  // prevent writes if remaining write count has reached zero
  }

  if (!(strcmp(cal.secret, secret) == 0)) {
    Serial.println(F("{\"ERROR\":\"cal_set -> wrong secret\"}"));
    return;  // don't set values if auth code does not match secret
  }

  if (cal.signature != WRITTEN_SIGNATURE) {
    Serial.println(F("{\"ERROR\":\"cal_set -> written signature does not match\"}"));
    return;  // don't set values if auth code does not match secret
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

  cal.writes -= 1;
  cal.valid = true;

  Serial.print(F("{\"INFO\":\"cal_set -> cal ok\",\"value\":\""));
  cal.data = calValue;
  Serial.print(cal.data);
  Serial.print("\"},\"writes_remaining\":");
  Serial.print(cal.writes);
  Serial.println("}");

  //cal_store.write(cal);
  EEPROM.put(EEPROM_START_ADDRESS, cal);
}

void secretObject::report_cal() {
  Serial.print("{\"INFO\":\"cal_report\",\"signature\":\"");
  Serial.print(cal.signature);
  Serial.print("\,\"secure\":");
  Serial.print(cal.secure);
  Serial.print("\",\"valid\":");
  Serial.print(cal.valid);
  Serial.print(",\"writes_left\":\"");
  Serial.print(cal.writes);
  Serial.print("\",\"cal-data\":\"");
  Serial.print(cal.data);
  Serial.println("\"}");
}