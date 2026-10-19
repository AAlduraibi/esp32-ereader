#include "reader.h"
#include "display.h"
#include "storage.h"
#include <string.h>

enum class Screen {
    Menu,
    Book
};

static Screen screen = Screen::Menu;    // which screen is showing 
static int cursor = 0;                  // which book the > is on
static int bookCount = 0;               // how many books there are
static int page = 0;                    // which page you're on in the open book

static void drawTitle(int x, int y, const char* name) {
    int maxWidth = 200 - x;

    if (displayTextWidth(name) <= maxWidth) {
        displayText(x, y, name);
        return;
    }

    char buf[140];
    int len = strlen(name);

    while (len > 0) {
        strlcpy(buf, name, len + 1);
        strcat(buf, "...");
        if (displayTextWidth(buf) <= maxWidth) {
            break;
        }
        len--;
    }

    displayText(x, y, buf);
}
static void drawMenu() {
    displayClear();

    if (bookCount == 0) {
        displayText(0, 15, "No books found");
        displayRefresh();
        return;
    }

    for (int i = 0; i < bookCount; i++) {
        int y = 15 + i * 20;
        if (i == cursor) {
            displayText(0, y, ">");
        }
        drawTitle(12, y, storageBookName(i));
    }
    displayRefresh();
}


void readerInit(){
    bookCount = storageListBooks();
    drawMenu();
}

void readerHandleButton(Button b){
    if (screen == Screen::Menu){
        if (b == Button::Next && cursor < bookCount - 1){
            cursor ++;
            drawMenu();
        }
        else if (b == Button::Prev && cursor > 0){
            cursor --;
            drawMenu();
        }
    }
}