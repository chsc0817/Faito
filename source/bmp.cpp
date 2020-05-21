#include "bmp.h"
#include <vector>
#include <unordered_map>


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

typedef std::vector<pos2> pixel_queue;

u32 index_of(image source, pos2 pos) {
    return pos.y * source.width + pos.x;
}

void propagate(image source, pos2 pos, u32 *map, pixel_queue *queue, u32 rect_index) {
    u32 index = index_of(source, pos);

    if (source.data[index * 4 + 3] == 0) {
        return;
    }
    map[index] = rect_index + 1;

    //left, right, top, bottom respectively
    pos2 addToVector[4] = {
        {pos.x - 1, pos.y},
        {pos.x + 1, pos.y},
        {pos.x, pos.y + 1},
        {pos.x, pos.y - 1}
    };

    for (u8 i = 0; i < 4; i++) {
        u32 index = index_of(source, addToVector[i]);
        if((addToVector[i].x >= 0) && (addToVector[i].x < source.width) &&
            (addToVector[i].y >= 0) && (addToVector[i].y < source.height) &&
            (map[index] == 0)) {
                queue->push_back(addToVector[i]);
        }
    }
}

bool find_pixels_for_single_sprite(image source, pos2 start, u32 *map, u32 rect_index){

    if (map[index_of(source, start)] != 0) {
        return false;
    }
    pixel_queue queue = {start};

    while (!queue.empty()) {
        auto next = queue.back();
        queue.pop_back();
        propagate(source, next, map, &queue, rect_index);
    }    

    return true;    
}

box2 *asd(u32 *box_count, image source) {
    u32 *map = new u32[source.width * source.height];
    memset(map, 0, sizeof(u32) * source.width * source.height); 

    u32 rect_index = 0;
    for (s32 y = 0; y < source.height; y++) {
        for (s32 x = 0; x < source.width; x++) {
            if (source.data[(y * source.width + x) * 4 + 3] != 0) {
                if (find_pixels_for_single_sprite(source, {x, y}, map, rect_index))
                    rect_index++;
            }
        }
    }

    box2 * boxes = new box2[rect_index];
    for (auto i = 0; i < rect_index; i++) {
        boxes[i].min = {(s32)source.width, (s32)source.height};
        boxes[i].max = {-1, -1};
    }

    for (s32 y = 0; y < source.height; y++) {
        for (s32 x = 0; x < source.width; x++) {
            u32 box_index = map[index_of(source, {x, y})] - 1;
            if (box_index == -1) 
                continue;
            
            if (boxes[box_index].min.x > x)
                boxes[box_index].min.x = x;

            if (boxes[box_index].max.x < x)
                boxes[box_index].max.x = x;

            if (boxes[box_index].min.y > y)
                boxes[box_index].min.y = y;

            if (boxes[box_index].max.y < y)
                boxes[box_index].max.y = y;
        }
    }

    *box_count = rect_index;
    delete[] map;
    return boxes;
}

texture LoadBMPTexture(u8_array source) {
    auto image = LoadBMP(source);
    auto texture = MakeTexture(image.width, image.height, image.data);
    
    return texture;
}