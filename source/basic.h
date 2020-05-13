#pragma once

#include <stdint.h>
#include <assert.h>


#define null NULL

#define s8  int8_t
#define s32 int32_t

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define f32 float

// maximal addressable size (32bit or 64bit), unsigned memory (umm)
#define umm size_t 

#define internal static

#define Swap(a, b) {auto temp = a; a = b; b = temp;} 

#define ArrayCountOf(static_array) (sizeof(static_array) / sizeof((static_array)[0]))   

// is 0 terminated c string
typedef char * cstring;

// c string "hello" => { 'h', 'e', 'l', 'l', 'o', 0 }

struct u8_array {
    u8 *base;
    umm count;

    u8 & operator[](umm index) {
        return base[index];
    }
};

internal u8_array Advance(u8_array *iterator, umm count = 1) {
    assert(iterator->count >= count);
    
    u8_array skipped_data;
    skipped_data.base = iterator->base;
    skipped_data.count = count;
    
    iterator->base  += count;
    iterator->count -= count;
    
    return skipped_data;
}