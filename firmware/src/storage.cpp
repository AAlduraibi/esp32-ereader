#include "storage.h"
#include "pins.h"
#include <SD.h>
#include <SPI.h>
#include <string.h>

constexpr int MAX_BOOKS = 50;
constexpr int MAX_NAME_LEN = 128;

static char bookNames[MAX_BOOKS][MAX_NAME_LEN];
static int bookCount = 0;
static File bookFile;

bool storageInit(){
    return SD.begin(SD_CS, SPI, 1000000);
}

static bool isBook(File& f){
    const char* ext = strrchr(f.name(), '.');
    if (f.isDirectory()){ return false;}
    if (f.name()[0] == '.'){ return false;}
    if (strlen(f.name()) >= MAX_NAME_LEN) { return false; }

    if (ext == nullptr){ return false;}
    else if (strcasecmp(ext, ".txt") == 0 || strcasecmp(ext, ".epub") == 0){
        return true;
    }
    return false;

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
        
        if (isBook(f)){
            strlcpy(bookNames[bookCount], f.name(), MAX_NAME_LEN);
            bookCount += 1;
        }
        f.close();
        f = dir.openNextFile();
    }
    f.close();
    dir.close();
    return bookCount;
}

const char* storageBookName(int index){
    if (index < 0 || index >= bookCount){
        return nullptr;
    }
    else{
        return bookNames[index];
    }
}

bool storageOpenBook(const char* name){
    bookFile.close();

    char path[140];
    snprintf(path, sizeof(path), "/books/%s", name);

    bookFile = SD.open(path);
    return bookFile;
}

int storageReadBook(char* buf, int maxLen){
    if (!bookFile) {
        buf[0] = '\0';
        return 0;
    }
    int num = bookFile.read((uint8_t*)buf, maxLen-1);
    buf[num] = '\0';
    return num;
}

void storageCloseBook(){
    bookFile.close();
}
