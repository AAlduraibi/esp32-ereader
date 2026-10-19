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
  buttonsInit();
}

void loop() {
  Button b = buttonsRead();

  switch (b) {
    case Button::Next:   Serial.println("next");   break;
    case Button::Prev:   Serial.println("prev");   break;
    case Button::Select: Serial.println("select"); break;
    case Button::Power:  Serial.println("power");  break;
    case Button::None:   break;
  }
}

