#pragma once

bool storageInit();

int storageListBooks();

const char* storageBookName(int index);

bool storageOpenBook(const char* name);

int storageReadBook(char* buf, int maxLen);

void storageCloseBook();