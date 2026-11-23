#include "epub.h"
#include <rom/miniz.h>
#include <stdint.h>
#include "storage.h"
#include <string.h>


static uint16_t cdCount = 0;
static uint32_t cdOffset = 0;

struct ZipEntry {
    uint16_t method;
    uint32_t compSize;
    uint32_t uncompSize;
    uint32_t localOffset;
};

static uint8_t inBuf[1024];

static uint8_t dict[TINFL_LZ_DICT_SIZE];

static tinfl_decompressor inflator;

static uint8_t outBuf[512];

static int outLen = 0;

static bool inTag = false;

static uint16_t rd16(const uint8_t* p){
    return p[0] | (p[1] << 8);
}

static uint32_t rd32(const uint8_t* p){
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool endsWith(const char* s, const char* end){
    int lenS = strlen(s);
    int lenE = strlen(end);
    if (lenE>lenS){
        return false;
    }
    return strcmp(end, s + (lenS-lenE)) == 0;
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

// TEST (stage 1): prints every file name in the zip
/*
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
*/

static bool findFile(const char* want, ZipEntry* out){
    uint32_t pos = cdOffset;
    for (int i=0; i < cdCount; i++){
    if (!storageSeek(pos)){ return false; }
    uint8_t h[46];
    if (storageReadBytes(h, 46) != 46){ return false;}
    if (rd32(h) != 0x02014b50){ return false;}
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
        if (strcmp(name, want) == 0){
            out->method = rd16(h + 10);
            out->compSize = rd32(h + 20);
            out->uncompSize = rd32(h + 24);
            out->localOffset = rd32(h + 42);
            return true;
        }
        pos += 46 + nameLen + extraLen + commentLen;
    }
    return false;
}

static uint32_t dataStart(const ZipEntry* e){
    if (!storageSeek(e->localOffset)){ return 0;}
    uint8_t lh[30];
    if (storageReadBytes(lh, 30) != 30){ return 0;}
    if (rd32(lh) != 0x04034b50){ return 0;}
    uint16_t nameLen = rd16(lh+26);
    uint16_t extraLen = rd16(lh+28);
    return e->localOffset + 30 + nameLen + extraLen;
}

static bool findChapter(ZipEntry* out){
    uint32_t pos = cdOffset;
    for (int i=0; i < cdCount; i++){
    if (!storageSeek(pos)){ return false; }
    uint8_t h[46];
    if (storageReadBytes(h, 46) != 46){ return false;}
    if (rd32(h) != 0x02014b50){ return false;}
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
        if (endsWith(name, ".xhtml") || endsWith(name, ".html") || endsWith(name, ".htm")){
            out->method = rd16(h + 10);
            out->compSize = rd32(h + 20);
            out->uncompSize = rd32(h + 24);
            out->localOffset = rd32(h + 42);
            Serial.println(name);
            return true;
        }
        pos += 46 + nameLen + extraLen + commentLen;
    }
    return false;
}

static bool flushOut(){
    if (outLen == 0){
        return true;
    }
    bool ok = storageWriteCache(outBuf, outLen);
    outLen = 0;
    return ok;
}

static bool outByte(uint8_t c){
    if (outLen == (int)sizeof(outBuf) && !flushOut()){ return false;}
    outBuf[outLen] = c;
    outLen += 1;
    return true;
}

static bool parseChunk(const uint8_t* data, size_t len){
    for (size_t i=0; i<len; i++){
        uint8_t c = data[i];
        if (inTag){
            if (c == '>'){
                inTag = false;
            }
            continue;
        }
        if (c == '<'){
            inTag = true;
            continue;
        }
        if (!outByte(c)){ return false;}
    }
    return true;
}

static bool inflateToCache(const ZipEntry* e){
    uint32_t start = dataStart(e);
    if (start == 0 || !storageSeek(start)){ return false;}
    tinfl_init(&inflator);
    inTag = false;
    outLen = 0;
    uint32_t left = e->compSize;
    size_t inPos = 0;
    size_t inAvail = 0;
    size_t dictPos = 0;
    while (true) {
        if (inAvail == 0 && left >0){
            int n = left;
            if (n > (int)sizeof(inBuf)){
                n = sizeof(inBuf);
            }
            int got = storageReadBytes(inBuf, n);
            if (got<= 0){
                Serial.println("read failed");
                return false;
            }
            left -= got;
            inAvail = got;
            inPos = 0;
        }
        size_t inSize = inAvail;
        size_t outSize = TINFL_LZ_DICT_SIZE - dictPos;
        int flags = 0;
        if (left>0){
            flags |= TINFL_FLAG_HAS_MORE_INPUT;
        }
        tinfl_status status = tinfl_decompress(&inflator, inBuf+inPos, &inSize, dict, dict+dictPos, &outSize, flags);
        inPos += inSize;
        inAvail -= inSize;
        if (!parseChunk(dict + dictPos, outSize)) { return false;}
        dictPos += outSize;
        if (dictPos == TINFL_LZ_DICT_SIZE){
            dictPos = 0;
        }
        if (status == TINFL_STATUS_DONE){
            break;
        }
        if (status < 0){
            Serial.println("inflate failed");
            return false;
        }
    }
    return flushOut();
}

bool epubTest(const char* name){
    if (!storageOpenBook(name)){ return false;}
    if (!readEOCD()){ 
        Serial.println("not a zip");
        return false;
    }
    Serial.println(cdCount);
    ZipEntry e;
    if (!findFile("mimetype", &e)){
        Serial.println("no mimetype");
        return false;
    }
    uint32_t start = dataStart(&e);
    if (start == 0 || !storageSeek(start)){
        Serial.println("bad local header");
        return false;
    }
    char buf[64];
    int n = e.compSize;
    if (n > 63){n=63;}
    int got = storageReadBytes((uint8_t*)buf, n);
    buf[got] = '\0';
    Serial.println(buf);
    if (!findChapter(&e)){
        Serial.println("no chapter");
        return false;
    }
    Serial.println(e.method);
    Serial.println(e.uncompSize);
    if (!storageOpenCache(name)){
        Serial.println("cache open failed");
        return false;
    }
    bool ok = inflateToCache(&e);
    storageCloseCache();
    if (!ok){
        Serial.println("inflate to cache failed");
        return false;
    }
    Serial.println("cache written");
    return true;
}