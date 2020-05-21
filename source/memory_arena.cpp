
#include "memory_arena.h"

u8_array allocate(memory_arena *arena, u32 byte_count, u32 byte_alignment)
{
    if (!byte_count)
        return {};
    
    assert(arena->used_count + byte_count <= arena->buffer.count);
    
    u8 *result = arena->buffer.base + arena->used_count;
    arena->used_count += byte_count;
    
    // x = a % b;
    // ( x + b - (a % b) ) % b == 0
    if ((u64)result % byte_alignment)
    {
        u32 align_padding = byte_alignment - ((u64)result % byte_alignment);
        
        assert(arena->used_count + align_padding <= arena->buffer.count);
        arena->used_count += align_padding;
        result            += align_padding;
    }
    
    return { result, byte_count };
}

// free in reverse order of allocations
void free_bytes(memory_arena *arena, u8 *data)
{
    if (!data)
        return; 
        
    assert((arena->buffer.base <= data) && (data < arena->buffer.base + arena->used_count));
    
    u64 index = (u64)(data - arena->buffer.base);
    arena->used_count = index;
}

void reallocate(memory_arena *arena, u8 **data, u32 byte_count, u32 byte_alignment)
{
    free_bytes(arena, *data);
    *data = allocate(arena, byte_count, byte_alignment).base;
}

void clear(memory_arena *arena)
{
    arena->used_count = 0;
}
