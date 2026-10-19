#include "storage.h"
#include "pins.h"
#include <SD.h>

constexpr int MAX_BOOKS = 50;
constexpr int MAX_NAME_LEN = 128;

static char bookNames[MAX_BOOKS][MAX_NAME_LEN];
static int bookCount = 0;

bool storageInit(){
    return SD.begin(SD_CS);
}

int storageListBooks(){
    bookCount = 0;
    File dir = SD.open("/books");
    if (!dir || !dir.isDirectory()){
        dir.close();
        return 0;
    } 
    File f = dir.openNextFile();
    while (f && bookCount < MAX_BOOKS){
        strlcpy(bookNames[bookCount], f.name(), MAX_NAME_LEN);
        bookCount += 1;
        f.close();
        f = dir.openNextFile();
    }
    f.close();
    dir.close();
    return bookCount;
}

