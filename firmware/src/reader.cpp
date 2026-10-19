#include "reader.h"
#include "display.h"
#include "storage.h"

enum class Screen {
    Menu,
    Book
};

static Screen screen = Screen::Menu;    // which screen is showing 
static int cursor = 0;                  // which book the > is on
static int bookCount = 0;               // how many books there are
static int page = 0;                    // which page you're on in the open book

static void drawMenu() {
    displayClear();

    if (bookCount == 0){
        displayText(0, 0, "No books found");
        displayRefresh();
        return;
    }

    for (int i = 0; i < bookCount; i++){
        int y = 15 + i * 20;
        if (i == cursor){
            displayText(0, y, ">");
        }
        displayText(12, y, storageBookName(i)); 
    }
    displayRefresh();
}

void readerInit(){
    bookCount = storageListBooks();
    drawMenu();
}