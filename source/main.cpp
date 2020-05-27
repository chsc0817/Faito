#pragma once

#include "basic.h"
#include "win32_platform.h"
#include "bmp.cpp"
#include "render.cpp"
#include "input.cpp"
#include "random.cpp"
#include "memory_arena.cpp"

const f32 TARGET_WIDTH_OVER_HEIGHT = 16.0f/9.0f;

struct sprite {
    box2s box;
    vec2s center;
};

struct game_state {
    memory_arena memory;
    memory_arena temporary_memory;

    win32_window window;
    render_context renderer;
    random_generator rng;
    texture king_sprites;
    
    sprite king_idle;
    sprite king_move;
    sprite king_punch;
    
    u32 king_sprite_count;
    box2s *king_sprite_boxes; // allocated from memory
};

texture LoadBMPTextureFromFile(win32_api *api, memory_arena *arena, cstring file_path) {
    auto result = api->read_entire_file(arena, file_path);
    
    assert(result.ok);
    texture my_texture = LoadBMPTexture(result.data);
    
    free_bytes(arena, result.data.base);
    
    return my_texture;
}

void DrawSprite(render_context *renderer, texture my_texture, vec2s position, sprite my_sprite) {
    DrawTexturedRect(renderer, my_texture, position - my_sprite.center, my_sprite.box, 0, 0);
}

void Reset(game_state *state)
{
    state->king_idle.box.min.x = 15;
    state->king_idle.box.max.x = state->king_idle.box.min.x + 61;
    state->king_idle.box.max.y = state->king_sprites.size.height - 1 - 9;
    state->king_idle.box.min.y = state->king_idle.box.max.y - 116;
    {
        auto size = state->king_idle.box.max - state->king_idle.box.min;
        state->king_idle.center.x = size.width / 2;
        state->king_idle.center.y = 6;
    }

    state->king_move.box.min.x = 105;
    state->king_move.box.max.x = state->king_move.box.min.x + 66;
    state->king_move.box.max.y = state->king_sprites.size.height - 1 - 158;
    state->king_move.box.min.y = state->king_move.box.max.y - 117;
    {
        auto size = state->king_move.box.max - state->king_move.box.min;
        state->king_move.center = {};
        state->king_move.center.x = size.width / 2;
        state->king_move.center.y = 6;
    }
}

INIT_DECLARATION {
    auto memory = api->make_memory_arena(api, 20 << 20); // 20 mb

    auto state = allocate_item(&memory, game_state);
    *state = {};
    
    state->temporary_memory = api->make_memory_arena(api, 128 << 20); // 20 mb
    auto tmemory = &state->temporary_memory;

    SYSTEMTIME system_time;
    GetSystemTime(&system_time);    
    FILETIME file_time;
    SystemTimeToFileTime(&system_time, &file_time);

    state->rng = MakeRandomGenerator(file_time.dwLowDateTime + file_time.dwHighDateTime);

    state->window;
    api->create_window(&state->window, api, 1280 * TARGET_WIDTH_OVER_HEIGHT, 1280, "Faito!");

    //state->king_sprites = LoadBMPTextureFromFile(api, "data/thekinga.bmp");
    {   
        auto result = api->read_entire_file(tmemory, "data/thekinga.bmp");
        assert(result.ok);
        //state->king_sprites = LoadBMPTexture(result.data);

        auto image = LoadBMP(result.data);
        state->king_sprites = MakeTexture(image.width, image.height, image.data);
   
        state->king_sprite_boxes = asd(&state->king_sprite_count, image, &memory, tmemory);
        free_bytes(tmemory, result.data.base);
    }

    state->renderer = MakeRenderContext(TARGET_WIDTH_OVER_HEIGHT);

    // make shure that all allocations form memory, are visible to state->memory
    state->memory = memory;
    
    Reset(state);
    
    return (u8 *) state;
}

UPDATE_DECLARATION {
    auto state = (game_state *) init_data;
    if ((state->window.width == 0) || (state->window.height == 0)) {
        return;
    }
    
    // THIS IS GARBAGE COLLECTION!!!
    auto tmemory = &state->temporary_memory;
    clear(tmemory);

    auto renderer = &state->renderer;
    if (api->code_was_reloaded) {
        ReloadRenderContext(renderer);
        Reset(state);
    }

    renderer->vertex_count = 0;

    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, state->window.width, state->window.height);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
    
    f32 current_width_over_height = state->window.width / (f32) state->window.height;

    u32 canvas_width, canvas_height;

    if (current_width_over_height > TARGET_WIDTH_OVER_HEIGHT) {
        canvas_height = state->window.height;
        canvas_width = canvas_height * TARGET_WIDTH_OVER_HEIGHT;
    } else {
        canvas_width = state->window.width;
        canvas_height = canvas_width / TARGET_WIDTH_OVER_HEIGHT;
    }	
    glEnable(GL_SCISSOR_TEST);

    glViewport((state->window.width - canvas_width) * 0.5f, (state->window.height - canvas_height) * 0.5f, canvas_width, canvas_height);
    glScissor((state->window.width - canvas_width) * 0.5f, (state->window.height - canvas_height) * 0.5f, canvas_width, canvas_height);
    glClearColor(0, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
#if 0
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 1/255.0f);
#endif
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    s32 ground_y = renderer->canvas_height_in_texels * -0.22f;

    //DrawRect(renderer, { (s32)-renderer->canvas_width_in_texels, ground_y }, 2 * canvas_width, canvas_height, Color32(0.1f, 0.8f, 0.01f), 0, 1);
    DrawSprite(renderer, state->king_sprites, { 0, ground_y }, state->king_move);
    
    {
        auto size = state->king_move.box.max - state->king_move.box.min;
        DrawRect(renderer, vec2s{ 0, ground_y } - state->king_move.center, size.width, size.height, Color32(0.1f, 0.8f, 0.01f, 0.2f), 0, 0);
    }
    
    DrawSprite(renderer, state->king_sprites, { 80, ground_y }, state->king_idle);
    
    {
        auto size = state->king_idle.box.max - state->king_idle.box.min;
        DrawRect(renderer, vec2s{ 80, ground_y } - state->king_idle.center, size.width, size.height, Color32(0.1f, 0.8f, 0.01f, 0.2f), 0, 0);
    }
    
#if 0
    DrawTexture(renderer, state->king_sprites, { 0, 0 }, 0.5f, 0.5f, 0, 0, Color32(1, 1, 1, 0.25f));
    
    for (u32 i = 0; i < state->king_sprite_count; i++) {
        auto box = state->king_sprite_boxes[i];
        auto size = box.max - box.min;

        DrawRect(renderer, box.min - state->king_sprites.size * 0.5f, size.width, size.height, Color32(1.0f, 0.0f, 0.0f, 0.25f), 0, 0);
        DrawTexturedRect(renderer, state->king_sprites, box.min - state->king_sprites.size * 0.5f, box, 0, 0);
    }
#endif

    Render(renderer);
    
    api->display_window(&state->window);    

}
