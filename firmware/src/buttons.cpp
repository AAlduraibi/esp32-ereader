#include "buttons.h"
#include "pins.h"
#include <Arduino.h>

void buttonsInit(){
    pinMode(BTN_POWER, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_NEXT, INPUT_PULLUP);
    pinMode(BTN_PREV, INPUT_PULLUP);
}

static bool nextWasPressed = false;
static bool prevWasPressed = false;
static bool powerWasPressed = false;
static bool selectWasPressed = false;

Button buttonsRead(){
    if (digitalRead(BTN_NEXT) == LOW){
        if (!nextWasPressed) {
            nextWasPressed = true;
            return Button::Next;
        }
    } else {
        nextWasPressed = false;
    }
    if (digitalRead(BTN_PREV) == LOW){
        if (!prevWasPressed){
            prevWasPressed = true;
            return Button::Prev;
        }
    } else {
        prevWasPressed = false;
    }
    if (digitalRead(BTN_SELECT) == LOW){
        if (!selectWasPressed){
            selectWasPressed = true;
            return Button::Select;
        }
    } else {
        selectWasPressed = false;
    }
    if (digitalRead(BTN_POWER) == LOW){
        if (!powerWasPressed){
            powerWasPressed = true;
            return Button::Power;
        }
    } else {
        powerWasPressed = false;
    }
    return Button::None;
}
