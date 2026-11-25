#include "reader.h"
#include "display.h"
#include "storage.h"
#include "power.h"
#include "epub.h"
#include <string.h>
#include <stdint.h>
#include <Arduino.h>


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
constexpr int MAX_PAGES = 1000;
constexpr unsigned long SLEEP_MS = 5*60*1000;
static unsigned long lastActivity = 0;
static uint32_t pageStarts[MAX_PAGES];
static bool lastPage = false;
static RTC_DATA_ATTR bool wasReading = false;
static RTC_DATA_ATTR int sleepBook = 0;


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
    displaySetStyle(Style::Normal);
    int y = 15;
    int lines = 0;
    int x = 0;
    bool space = false;
    int i = 0;
    while (text[i] != '\0' && lines <PAGE_LINES){
        if (text[i] == '\r' || text[i] == ' '){
            i++;
            space = true;
        }
        else if (text[i] == '\n'){
            y += 20;
            lines +=1;
            x = 0;
            space = false;
            i++;
        }
        else if (text[i] == MARK_BOLD){
            displaySetStyle(Style::Bold);
            i++;
        }
        else if (text[i] == MARK_ITALIC){
            displaySetStyle(Style::Italic);
            i++;
        }
        else if (text[i] == MARK_NORMAL){
            displaySetStyle(Style::Normal);
            i++;
        }
        else{
            int j = i;
            while (text[j] != ' ' && text[j] != '\n' && text[j] != '\r' && text[j] != '\0' && text[j] != MARK_BOLD && text[j] != MARK_ITALIC && text[j] != MARK_NORMAL){
                j++;
            }
            char word[100] = "";
            int n = j-i;
            if (n > (int)sizeof(word) - 1){
                n = (int)sizeof(word) -1;
            }
            strncat(word, text+i, n);
            int gap = 0;
            if (space && x>0){
                gap = displayTextAdvance(" ");
            }
            if (x + gap + displayTextAdvance(word) <= 200){
                x += gap;
                displayText(x, y, word);
                x += displayTextAdvance(word);
                space = false;
                i = j;
            }
            else{
                if (x == 0){
                    char test[100];
                    int k = j-i;
                    if (k > (int)sizeof(test)-2){
                        k = (int)sizeof(test)-2;
                    }
                    while (k>0){
                        test[0] = '\0';
                        strncat(test,text + i,k);
                        strlcat(test,"-",sizeof(test));
                        if (displayTextWidth(test) <= 200){
                            break;
                        }
                        k--;
                    }
                    if (k == 0) k = 1;
                    displayText(0,y,test);
                    y += 20;
                    lines += 1;
                    i += k;
                    space = false;
                }
                else{
                    y += 20;
                    lines +=1;
                    x = 0;
                    space = false;
                }
                
            }
        }
    }
    displaySetStyle(Style::Normal);
    return i;
}

static void drawPage(){
    char buf[400];
    displayClear();
    storageSeek(pageStarts[page]);
    int got = storageReadBook(buf, sizeof(buf));
    int used = drawWrapped(buf);
    if (buf[used] == '\0' && got < (int)sizeof(buf) - 1){
        lastPage = true;
    }
    else{
        lastPage = false;
    }
    if (page + 1 < MAX_PAGES){
        pageStarts[page + 1] = pageStarts[page] + used;
    }
    storageSavePos(storageBookName(cursor), pageStarts[page]);
    displayRefresh();
}

static bool openBook(){
    const char* name = storageBookName(cursor);
    if (epubIsEpub(name)){
        if (!storageOpenCacheRead(name)){
            if(!epubBuildCache(name)){
                return false;
            }
            if (!storageOpenCacheRead(name)){ return false;}
        }
    }
    else{
        if (!storageOpenBook(name)) { return false;}
    }
    screen = Screen::Book;
    page = 0;
    pageStarts[0] = storageLoadPos(name);
    drawPage();
    return true;
}

static void goToSleep(bool white){
    wasReading = (screen == Screen::Book);
    sleepBook = cursor;
    displayClear();
    if (!white){
        displayText((200-displayTextWidth("Sleeping"))/2, 100, "Sleeping");
    }
    displayRefresh();
    displaySleep();
    powerSleep();
}


void readerInit(){
    bookCount = storageListBooks();
    if (wasReading && sleepBook < bookCount){
        cursor = sleepBook;
        if (openBook()){return;}
    }
    drawMenu();
}

void readerHandleButton(Button b){
    lastActivity = millis();
    if (b == Button::Power){
        goToSleep(false);
    }
    if (b == Button::PowerLong){
        goToSleep(true);
    }
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
            openBook();
        }
    }
    else if (screen == Screen::Book){
        if (b == Button::Next && page + 1 < MAX_PAGES && !lastPage){
            page++;
            drawPage();
        }
        else if (b == Button::Prev && page > 0){
            page--;
            drawPage();
        }
        else if (b == Button::Select){
            storageCloseBook();
            screen = Screen::Menu;
            drawMenu();
        }
    }
}

void readerUpdate(){
    if ((millis() - lastActivity) > SLEEP_MS){
        goToSleep(false);
    }
}