#pragma once

enum class Button{
    None,
    Power,
    Next,
    Prev,
    Select
};

void buttonsInit();

Button buttonsRead();