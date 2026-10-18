#include <Arduino.h>
#include "display.h"
#include "buttons.h"
#include "storage.h"
#include "reader.h"
#include "battery.h"

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>
#include <SD.h>

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
  GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));


void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, EPD_CS);
  display.init(115200);
  display.setRotation(0);

  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("SD card failed!");
  }
  pinMode(BTN_POWER, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
}

bool nextWasPressed = false;
bool prevWasPressed = false;
bool selectWasPressed = false;
bool powerWasPressed = false;

void loop(){
  if (digitalRead(BTN_NEXT) == LOW) {
    if (!nextWasPressed) {
      Serial.println("next pressed");
      nextWasPressed = true;
    }
  } else {
    nextWasPressed = false;
  }

  if (digitalRead(BTN_PREV) == LOW) {
    if (!prevWasPressed) {
      Serial.println("prev pressed");
      prevWasPressed = true;
    }
  } else {
    prevWasPressed = false;
  }

  if (digitalRead(BTN_SELECT) == LOW) {
    if (!selectWasPressed) {
      Serial.println("select pressed");
      selectWasPressed = true;
    }
  } else {
    selectWasPressed = false;
  }

  if (digitalRead(BTN_POWER) == LOW) {
    if (!powerWasPressed) {
      Serial.println("power pressed");
      powerWasPressed = true;
    }
  } else {
    powerWasPressed = false;
  }
}

void listBooks() {
  File dir = SD.open("/books");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    Serial.println(entry.name());
    entry.close();
  }
  dir.close();
}