#include "secretObject.h"

secretObject memory;

secretObject::calStruc global_cal;

secretObject::settingsStruc global_settings;



void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(1);  // give time for Serial object to start
  }
  Serial.println("begin");
  delay(5000);

  Serial.println("STEP 1");
  global_settings = memory.get_settings();
  delay(10000);

  Serial.println("STEP 2");
  global_cal = memory.get_cal();
  delay(10000);

  Serial.println("STEP 3");
  memory.set_load_max(10, "deezenz");
  delay(10000);

  Serial.println("STEP 4");
  memory.set_secret("deeznutz");
  delay(10000);

  Serial.println("STEP 5");
  memory.set_material("steel", "deeznutz");
  delay(10000);

  Serial.println("STEP 6");
  memory.set_diameter("8mm", "deeznutz");
  delay(10000);

  Serial.println("STEP 7");
  global_settings = memory.get_settings();
  delay(10000);

  Serial.println("STEP 8");
  memory.set_angle_max(444, "deeznutz");
  delay(10000);

  Serial.println("STEP 9");
  memory.set_load_max(666, "deeznutz");
  delay(10000);

  Serial.println("STEP 10");
  global_settings = memory.get_settings();
  delay(10000);


  Serial.println("STEP 11");
  memory.set_cal_value(5830, "deeznutz");
  delay(10000);

  Serial.println("STEP 12");
  global_cal = memory.get_cal();
}

void loop() {
}
