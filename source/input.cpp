#include "input.h"

void ButtonUpdate(button *b, bool is_active) {
    if (b->half_transition_count == 63) {
        b->half_transition_count_overflow = true;
    }
    
    b->half_transition_count++;
    b->is_active = is_active;
}

bool WasPressed(button b) {
    //if b.is_active && transition >= 1
    //if !b.is_active && transition >= 2
    return (b.half_transition_count >= 2 - b.is_active) || b.half_transition_count_overflow; 
}

bool WasReleased(button b) {
    //if b.is_active && transition >= 2
    //if !b.is_active && transition >= 1
    return (b.half_transition_count >= 1 + b.is_active) || b.half_transition_count_overflow; 
}