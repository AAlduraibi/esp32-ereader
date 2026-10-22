#pragma once
#include <stdint.h>

bool storageInit();

int storageListBooks();

const char* storageBookName(int index);

bool storageOpenBook(const char* name);

int storageReadBook(char* buf, int maxLen);

void storageCloseBook();

bool storageSeek(uint32_t pos);