#pragma once

#include "basic.h"

struct texture {
	s32 width, height;
	u32 id;
};

struct sprite_array {
	texture sprites[256];
	s32 length;
};

union rgba32{
	struct {
		u8 r, g, b, a;
	};

	struct {
		u8 red, green, blue, alpha;
	};

	u8 values[4];
};

struct pos2{
    s32 x,y;
};

struct box2 {
	pos2 min, max;
};

#pragma pack(push, 1)
struct render_vertex {
	f32 x, y, z;
	f32 u, v;
	rgba32 color;
};
#pragma pack(pop)

struct render_command {
	u32 texture_id;
	u32 vertex_offset, vertex_count;
};

struct render_context {
	render_vertex vertices[4096];
	render_command commands[256];
	u32 vertex_count;
	u32 command_count;
	u32 program;
	s32 diffuse_texture_uniform;
	texture white_texture;
	f32 target_width_over_height;
	f32 canvas_width_in_texels, canvas_height_in_texels;
	f32 canvas_texel_width, canvas_texel_height;
};

texture MakeTexture (s32 width, s32 height, u8_array pixels);

rgba32 Color32(f32 r, f32 g, f32 b, f32 a = 1.0f);

const f32 DEFAULT_CANVAS_WIDTH_IN_TEXELS = 256.0f;
const f32 DEFAULT_X_ALIGNMENT = 0.5f;
const f32 DEFAULT_Y_ALIGNMENT = 0.0f;
const f32 DEFAULT_FLIP = 0.0f;
const rgba32 DEFAULT_COLOR = Color32(1.0f, 1.0f, 1.0f, 1.0f);

render_context MakeRenderContext(f32 target_width_over_height, f32 canvas_width_in_texels = DEFAULT_CANVAS_WIDTH_IN_TEXELS);
void DrawTexturedRect(render_context *renderer, texture my_texture, f32 x, f32 y, f32 tx, f32 ty, f32 width, f32 height, f32 x_alignment = DEFAULT_X_ALIGNMENT, f32 y_alignment = DEFAULT_Y_ALIGNMENT, f32 x_flip = DEFAULT_FLIP, f32 y_flip = DEFAULT_FLIP, rgba32 color = DEFAULT_COLOR, f32 z = 0);
void DrawRect(render_context *renderer, f32 x, f32 y, f32 width, f32 height, rgba32 color = DEFAULT_COLOR, f32 x_alignment = 0, f32 y_alignment = DEFAULT_Y_ALIGNMENT, f32 z = 0);
void DrawTexture(render_context *renderer, texture my_texture, f32 x, f32 y, f32 x_alignment = DEFAULT_X_ALIGNMENT, f32 y_alignment = DEFAULT_Y_ALIGNMENT, f32 x_flip = DEFAULT_FLIP, f32 y_flip = DEFAULT_FLIP, rgba32 color = DEFAULT_COLOR, f32 z = 0);
void Render(render_context *renderer);