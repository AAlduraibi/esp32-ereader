#pragma once

enum class Button{
    None,
    Power,
    PowerLong,
    Next,
    Prev,
    Select
};

void buttonsInit();

Button buttonsRead();