#include "display.h"
#include "pins.h"
#include <GxEPD2_BW.h>
#include <SPI.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

static GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
  GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

static const GFXfont* curFont = &FreeSerif9pt7b;

void displayInit() {
    SPI.begin(18, 19, 23);
    display.init(115200);
    display.setRotation(3);
    display.setFont(&FreeSerif9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setTextWrap(false);   
}

void displayClear() {
    display.fillScreen(GxEPD_WHITE);
}

void displayText(int x, int y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}

void displayRefresh() {
    display.display();
}

void displaySetStyle(Style style) {
    switch (style){
        case Style::Normal:
            curFont = &FreeSerif9pt7b;
            break;
        case Style::Bold:
            curFont = &FreeSerifBold9pt7b;
            break;
        case Style::Italic:
            curFont = &FreeSerifItalic9pt7b;
            break;
    }
    display.setFont(curFont);
}

int displayTextWidth(const char* text) {
    int16_t x1, y1;     // the library fills these in, we don't need them
    uint16_t w, h;      // w = width, h = height

    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    return w;         
}

int displayTextAdvance(const char* text){
    int total = 0;
    for (int i = 0; text[i] != '\0'; i++){
        uint8_t c = (uint8_t)text[i];
        if (c >= curFont->first && c <= curFont->last){
            total += curFont->glyph[c - curFont->first].xAdvance;
        }
    }
    return total;
}

void displaySleep(){
    display.hibernate();
}