#pragma once

#include "basic.h"

struct memory_arena
{
    u8_array buffer;
    u32 used_count;
};

#define allocate_item(arena, type)            ( (type *)allocate(arena, sizeof(type), alignof(type)).base )
#define allocate_items(arena, type, count)    ( (type *)allocate(arena, sizeof(type) * (count), alignof(type)).base )
#define reallocate_items(arena, items, count) reallocate(arena, (u8 **)items, sizeof(decltype(**items)) * (count), alignof(decltype(**(items)))) 

// info:
// u32 *foo;
// decltype(*foo) bar; -> u32 bar; replace with type of expression (copy paste)


#define free_items(arena, data)               free_bytes(arena, (u8 *)data)
#define free_item(arena, data)                free_items(arena, data)

u8_array allocate(memory_arena *arena, u32 byte_count, u32 byte_alignment = 16);
// free in reverse order of allocations
void free_bytes(memory_arena *arena, u8 *data);
void reallocate(memory_arena *arena, u8 **data, u32 byte_count, u32 byte_alignment);

void clear(memory_arena *arena);