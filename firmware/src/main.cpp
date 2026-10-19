#include <Arduino.h>
#include "display.h"
#include "buttons.h"
#include "storage.h"
#include "reader.h"
#include "battery.h"

void setup(){
  Serial.begin(115200);
  displayInit();
  displayClear();
  displayText(2,25, "i want to do this");
  displayRefresh();
}

void loop() {}