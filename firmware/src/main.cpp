#include <Arduino.h>
#include "display.h"
#include "buttons.h"
#include "storage.h"
#include "reader.h"
#include "battery.h"
#include "epub.h"


void setup(){
  Serial.begin(115200);
  displayInit();
  
  buttonsInit();
  
  if (!storageInit()){
    Serial.println("SD Init failed");
    displayClear();
    displayText(0, 15, "No SD card");
    displayRefresh();
  }
  else{
    epubTest("The Way of Kings (Brandon Sanderson).epub");
    readerInit();
  }
}

void loop() {
  Button b = buttonsRead();
  if (b != Button::None){
    readerHandleButton(b);
  }
  readerUpdate();
}

