#include "epub.h"
#include <rom/miniz.h>
#include <stdint.h>
#include "storage.h"


static uint16_t cdCount = 0;
static uint32_t cdOffset = 0;

static uint16_t rd16(const uint8_t* p){
    return p[0] | (p[1] << 8);
}

static uint32_t rd32(const uint8_t* p){
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool readEOCD(){
    uint32_t size = storageBookSize();
    if (size < 22){ return false;}
    if (!storageSeek(size - 22)){ return false;}
    uint8_t eocd[22];
    if (storageReadBytes(eocd, 22) != 22){ return false;}
    if (rd32(eocd) != 0x06054b50){ return false;}
    cdCount = rd16(eocd + 10);
    cdOffset = rd32(eocd + 16);
    return true;
}

static void listFiles(){
    uint32_t pos = cdOffset;
    for (int i=0; i < cdCount; i++){
        if (!storageSeek(pos)){ return; }
        uint8_t h[46];
        if (storageReadBytes(h, 46) != 46){ return;}
        if (rd32(h) != 0x02014b50){ return;}
        uint16_t nameLen = rd16(h + 28);
        uint16_t extraLen = rd16(h + 30);
        uint16_t commentLen = rd16(h + 32);
        char name[128];
        int n = nameLen;
        if (n > (int)sizeof(name) - 1){
            n = sizeof(name) - 1;
        }
        storageReadBytes((uint8_t*)name, n);
        name[n] = '\0';
        Serial.println(name);
        pos += 46 + nameLen + extraLen + commentLen;
    }
}

bool epubTest(const char* name){
    if (!storageOpenBook(name)){ return false;}
    if (!readEOCD()){ 
        Serial.println("not a zip");
        return false;
    }
    Serial.println(cdCount);
    listFiles();
    return true;
}