#include "epub.h"
#include <rom/miniz.h>
#include <stdint.h>


static uint16_t rd16(const uint8_t* p){
    return p[0] | (p[1] << 8);
}

static uint32_t rd32(const uint8_t* p){
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}