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
constexpr int MENU_LINES = 9;
static int top = 0;
constexpr int PAGE_LINES = 9;


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
    if (cursor < top){
        top = cursor;
    }
    if (cursor >= top + MENU_LINES){
        top = cursor - MENU_LINES + 1;
    }
    for (int i = top; i < top + MENU_LINES && i < bookCount; i++) {
        int y = 15 + (i-top) * 20;
        if (i == cursor) {
            displayText(0, y, ">");
        }
        drawTitle(12, y, storageBookName(i));
    }
    displayRefresh();
}

static int drawWrapped(const char* text){
    char line[100] = "";
    int y = 15;
    int lines = 0;
    int i = 0;
    while (text[i] != '\0' && lines <PAGE_LINES){
        if (text[i] == '\r' || text[i] == ' '){
            i++;
        }
        else if (text[i] == '\n'){
            displayText(0,y,line);
            y += 20;
            lines +=1;
            line[0] = '\0';
            i++;
        }
        else{
            int j = i;
            while (text[j] != ' ' && text[j] != '\n' && text[j] != '\r' && text[j] != '\0'){
                j++;
            }
            char test[100];
            strlcpy(test, line, sizeof(test));
            if (line[0] != '\0'){
                strlcat(test, " ", sizeof(test));
            }
            strncat(test, text+i, j-i);
            if (displayTextWidth(test) <= 200){
                strlcpy(line, test, sizeof(line));
                i = j;
            }
            else{
                displayText(0,y,line);
                y += 20;
                lines +=1;
                line[0] = '\0';
            }
        }
    }
    if (line[0] != '\0' && lines <PAGE_LINES){
        displayText(0, y, line);

    }
    return i;
}

static void drawPage(){
    char buf[400];
    displayClear();
    storageReadBook(buf, sizeof(buf));
    drawWrapped(buf);
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
        else if (b == Button::Select && bookCount > 0){
            bool book = storageOpenBook(storageBookName(cursor));
            if (!book) { return; }
            screen = Screen::Book;
            page = 0;
            drawPage();
        }
    }
}