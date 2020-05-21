
#include "bmp.h"

image LoadBMP(u8_array source)
{
    auto it = source;
    
    auto header = NextItem(&it, bmp_header);
    
    auto dip_header_byte_count = *PeekItem(it, u32);
    
    assert(dip_header_byte_count == 124);
    
    auto dip_header = NextItem(&it, bmp_dip_header);
    
    // no compression
    assert(dip_header->compression_method == BMP_NO_COMPRESSION);
    
    Advance(&it, dip_header->dip_header_byte_count - sizeof(bmp_dip_header));
    
    image result;
    result.width = dip_header->width;
    result.height = dip_header->height;
    
    result.data = it;
    
    //bmp format is BGRA. convert to RGBA
    for(u32 i = 0; i < result.width * result.height; i++) {
        Swap(result.data[i * 4], result.data[i * 4 + 2]); 
    }
    
    return result;
}

u32 index_of(image source, pos2 pos) {
    return pos.y * source.width + pos.x;
}

struct pixel_queue
{
    pos2 *base;
    u32 count;
};

void propagate(image source, pos2 pos, u32 *map, pixel_queue *queue, u32 box_index, memory_arena *tmemory) {
    u32 index = index_of(source, pos);

    assert(source.data[index * 4 + 3] != 0);
    
    //left, right, top, bottom respectively
    pos2 addToVector[4] = {
        {pos.x - 1, pos.y},
        {pos.x + 1, pos.y},
        {pos.x, pos.y + 1},
        {pos.x, pos.y - 1}
    };

    for (u8 i = 0; i < 4; i++) {
        auto pos = addToVector[i];
        
        u32 index = index_of(source, pos);
        if((pos.x >= 0) && (pos.x < source.width) &&
            (pos.y >= 0) && (pos.y < source.height) &&
            (map[index] == 0) && (source.data[index * 4 + 3] != 0)) {
                map[index] = box_index + 1;
                reallocate_items(tmemory, &queue->base, ++queue->count);
                queue->base[queue->count - 1] = pos;
        }
    }
}

bool find_pixels_for_single_sprite(image source, pos2 start, u32 *map, u32 box_index, memory_arena *tmemory){

    auto index = index_of(source, start);
    
    if (map[index] != 0)
        return false;
        
    pixel_queue queue = { allocate_items(tmemory, pos2, 1), 1 };
    queue.base[0] = start;
    map[index] = box_index + 1;

    while (queue.count) {
        auto next = queue.base[queue.count - 1];
        reallocate_items(tmemory, &queue.base, --queue.count);
        
        propagate(source, next, map, &queue, box_index, tmemory);
    }    

    return true;    
}

box2 * asd(u32 *out_box_count, image source, memory_arena *box_memory, memory_arena *tmemory) {
    u32 *map = allocate_items(tmemory, u32, source.width * source.height);
    memset(map, 0, sizeof(u32) * source.width * source.height); 

    u32 box_count = 0;
    for (s32 y = 0; y < source.height; y++) {
        for (s32 x = 0; x < source.width; x++) {
            if (source.data[(y * source.width + x) * 4 + 3] != 0) {
                if (find_pixels_for_single_sprite(source, {x, y}, map, box_count, tmemory))
                    box_count++;
            }
        }
    }

    box2 * boxes = allocate_items(box_memory, box2, box_count);
    for (auto i = 0; i < box_count; i++) {
        boxes[i].min = {(s32)source.width + 1, (s32)source.height + 1};
        boxes[i].max = {-1, -1};
    }

    for (s32 y = 0; y < source.height; y++) {
        for (s32 x = 0; x < source.width; x++) {
            u32 box_index = map[index_of(source, {x, y})] - 1;
            if (box_index == -1) 
                continue;
            
            // max is + 1, since it is excluded
            
            if (boxes[box_index].min.x > x)
                boxes[box_index].min.x = x;

            if (boxes[box_index].max.x <= x)
                boxes[box_index].max.x = x + 1;

            if (boxes[box_index].min.y > y)
                boxes[box_index].min.y = y;

            if (boxes[box_index].max.y <= y)
                boxes[box_index].max.y = y + 1;
        }
    }

    *out_box_count = box_count;
    free_items(tmemory, map);
    return boxes;
}

texture LoadBMPTexture(u8_array source) {
    auto image = LoadBMP(source);
    auto texture = MakeTexture(image.width, image.height, image.data);
    
    return texture;
}