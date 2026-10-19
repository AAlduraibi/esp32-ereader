#pragma once

enum class Style {
    Normal,
    Bold,
    Italic
};

void displayInit();

void displayClear();

void displayText(int x, int y, const char* text);

void displayRefresh();

void displaySetStyle(Style style);

int displayTextWidth(const char* text);