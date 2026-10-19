#include "buttons.h"
#include "pins.h"
#include <Arduino.h>

//initial press
static bool nextWasPressed = false;
static bool prevWasPressed = false;
static bool powerWasPressed = false;
static bool selectWasPressed = false;

//debounce
constexpr unsigned long DEBOUNCE_MS = 30;
static unsigned long nextLastTime = 0;
static unsigned long prevLastTime = 0;
static unsigned long powerLastTime = 0;
static unsigned long selectLastTime = 0;

void buttonsInit(){
    pinMode(BTN_POWER, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_NEXT, INPUT_PULLUP);
    pinMode(BTN_PREV, INPUT_PULLUP);
}

Button buttonsRead(){
    //next
    if (millis() - nextLastTime > DEBOUNCE_MS) {
        if (digitalRead(BTN_NEXT) == LOW) {
            if (!nextWasPressed) {
                nextWasPressed = true;
                nextLastTime = millis();
                return Button::Next;
            }
        } else if (nextWasPressed) {
            nextWasPressed = false;
            nextLastTime = millis();
        }
    }

    //prev
    if (millis() - prevLastTime > DEBOUNCE_MS) {
        if (digitalRead(BTN_PREV) == LOW) {
            if (!prevWasPressed) {
                prevWasPressed = true;
                prevLastTime = millis();
                return Button::Prev;
            }
        } else if (prevWasPressed) {
            prevWasPressed = false;
            prevLastTime = millis();
        }
    }

    //select
    if (millis() - selectLastTime > DEBOUNCE_MS) {
        if (digitalRead(BTN_SELECT) == LOW) {
            if (!selectWasPressed) {
                selectWasPressed = true;
                selectLastTime = millis();
                return Button::Select;
            }
        } else if (selectWasPressed) {
            selectWasPressed = false;
            selectLastTime = millis();
        }
    }

    //power
    if (millis() - powerLastTime > DEBOUNCE_MS) {
        if (digitalRead(BTN_POWER) == LOW) {
            if (!powerWasPressed) {
                powerWasPressed = true;
                powerLastTime = millis();
                return Button::Power;
            }
        } else if (powerWasPressed) {
            powerWasPressed = false;
            powerLastTime = millis();
        }
    }

    return Button::None;
}