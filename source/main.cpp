#pragma once

#include "basic.h"
#include "win32_platform.h"
#include "bmp.cpp"
#include "render.cpp"
#include "input.cpp"
#include "random.cpp"

const f32 target_width_over_height = 16.0f/9.0f;

struct game_state {
	win32_window window;
    render_context renderer;
    random_generator rng;
};

texture LoadBMPTextureFromFile(win32_api *api, cstring file_path) {
    auto result = api->read_entire_file(file_path);
    assert(result.ok);
    texture my_texture = LoadBMPTexture(result.data);
    api->free_file_data(result.data);
    
    return my_texture;
}

INIT_DECLARATION {
	auto state = new game_state;
    *state = {};

    SYSTEMTIME system_time;
    GetSystemTime(&system_time);    
    FILETIME file_time;
    SystemTimeToFileTime(&system_time, &file_time);

    state->rng = MakeRandomGenerator(file_time.dwLowDateTime + file_time.dwHighDateTime);

	state->window;
	api->create_window(&state->window, api, 1280 * target_width_over_height, 1280, "Faito!");

    state->renderer = MakeRenderContext();

	return (u8 *) state;
}

UPDATE_DECLARATION {
	auto state = (game_state *) init_data;
	if ((state->window.width == 0) || (state->window.height == 0)) {
		return;
	}

    auto renderer = &state->renderer;
    renderer->vertex_count = 0;

	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, state->window.width, state->window.height);
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
	f32 current_width_over_height = state->window.width / (f32) state->window.height;

	u32 canvas_width, canvas_height;

	if (current_width_over_height > target_width_over_height) {
		canvas_height = state->window.height;
		canvas_width = canvas_height * target_width_over_height;
	} else {
		canvas_width = state->window.width;
		canvas_height = canvas_width / target_width_over_height;
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

    Render(renderer);
	
    api->display_window(&state->window);    

}
