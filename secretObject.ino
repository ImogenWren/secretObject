#include "secretObject.h"

secretObject memory;

secretObject::calStruc global_cal;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(1);  // give time for Serial object to start
  }
  Serial.println("begin");
  global_cal = memory.get_cal();


  memory.cal_set_secret("deeznutz");
  global_cal = memory.get_cal();

  memory.cal_set_values(5830, "deeznutz");

  global_cal = memory.get_cal();

}

void loop() {
}
