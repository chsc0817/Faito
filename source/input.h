#pragma once
#include "basic.h"

struct button {
    u8 is_active : 1;
    u8 half_transition_count_overflow : 1;
    u8 half_transition_count : 6;
};

struct user_input {
    button keys[256];    
};

bool WasPressed(button b);
bool WasReleased(button b);
void ButtonUpdate(button *b, bool is_active);