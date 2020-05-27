#include <gl/gl.h>
#include <gl/glext.h>
#include "render.h"

#pragma comment(lib, "opengl32.lib")

#define GL_DEBUG_CALLBACK(name) void name (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)

typedef GL_DEBUG_CALLBACK((APIENTRY * DEBUGPROC));


typedef void (*glGenBuffers_function) (GLsizei n, GLuint * buffers);
typedef void (*glBindBuffer_function) (GLenum target, GLuint buffer);
typedef void (*glDeleteBuffers_function) (GLsizei n, const GLuint * buffers);
typedef void (*glBufferData_function) (GLenum target, GLsizeiptr size, const void * data, GLenum usage);
typedef void (*glVertexAttribPointer_function) (GLuint index,
											  	GLint size,
											  	GLenum type,
											  	GLboolean normalized,
											  	GLsizei stride,
											  	const void * pointer
											  	);
typedef void (*glEnableVertexAttribArray_function) (GLuint index);
typedef void (*glDisableVertexAttribArray_function) (GLuint index);
//typedef void (*glTexCoordPointer_function) (GLint size, GLenum type, GLsizei stride, const void * pointer);
//typedef void (*glColorPointer_function) (GLint size, GLenum type, GLsizei stride, const void * pointer);
typedef void (*glGenVertexArrays_function) (GLsizei n, GLuint *arrays);
typedef void (*glBindVertexArray_function)(GLuint array);
typedef void (*glDeleteVertexArrays_function) (GLsizei n, const GLuint *arrays);
//typedef void (*glDrawArrays_function) (GLenum mode, GLint first,	GLsizei count);
typedef void (*glDebugMessageCallback_function)(DEBUGPROC callback, void * userParam);
typedef GLuint (*glCreateProgram_function)();
typedef void (*glAttachShader_function)(GLuint program,	GLuint shader);
typedef GLuint (*glCreateShader_function)(GLenum shaderType);
typedef void (*glDeleteProgram_function)(GLuint program);
typedef void (*glLinkProgram_function)(GLuint program);
typedef void (*glDeleteShader_function)(GLuint shader);
typedef void (*glUseProgram_function)(GLuint program);
typedef void (*glBindAttribLocation_function)(GLuint program, GLuint index, const GLchar *name);
typedef void (*glUniform1i_function)(GLint location, GLint v0);
typedef void (*glShaderSource_function)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void (*glCompileShader_function)(GLuint shader);
typedef void (*glGetShaderiv_function)(GLuint shader, GLenum pname, GLint *params);
typedef void (*glGetProgramiv_function)(GLuint program, GLenum pname, GLint *params);
typedef GLint (*glGetUniformLocation_function)(GLuint program, const GLchar *name);
typedef void (*glActiveTexture_function)(GLenum texture);

#define GlLoad(name) name = (name ##_function) wglGetProcAddress(#name); assert(name)

glGenBuffers_function glGenBuffers;
glBindBuffer_function glBindBuffer;
glDeleteBuffers_function glDeleteBuffers;
glBufferData_function glBufferData;
glVertexAttribPointer_function glVertexAttribPointer;
glEnableVertexAttribArray_function glEnableVertexAttribArray;
glDisableVertexAttribArray_function glDisableVertexAttribArray;
//glTexCoordPointer_function glTexCoordPointer;
//glColorPointer_function glColorPointer;
glGenVertexArrays_function glGenVertexArrays;
glBindVertexArray_function glBindVertexArray;
glDeleteVertexArrays_function glDeleteVertexArrays;
//glDrawArrays_function glDrawArrays;
glDebugMessageCallback_function glDebugMessageCallback;
glCreateProgram_function glCreateProgram;
glAttachShader_function glAttachShader;
glCreateShader_function glCreateShader;
glDeleteProgram_function glDeleteProgram;
glLinkProgram_function glLinkProgram;
glDeleteShader_function glDeleteShader;
glUseProgram_function glUseProgram;
glBindAttribLocation_function glBindAttribLocation;
glUniform1i_function glUniform1i;
glShaderSource_function glShaderSource;
glCompileShader_function glCompileShader;
glGetShaderiv_function glGetShaderiv;
glGetProgramiv_function glGetProgramiv;
glGetUniformLocation_function glGetUniformLocation;
glActiveTexture_function glActiveTexture;

enum {
	Vertex_Position,
	Vertex_Texture_Coordinate,
	Vertex_Color
};


GL_DEBUG_CALLBACK(GlHandleDebugMessage) {
	assert(0);
}

u32 MakeShaderObject(u32 shader_type, cstring source_code) {
	u32 shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, (const GLchar **)&source_code, null);
	glCompileShader(shader);

	s32 is_compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
	assert(is_compiled);

	return shader;
}

u32 MakeProgramObject(u32 vertex_shader_object, u32 fragment_shader_object) {
	u32 program = glCreateProgram();
	glAttachShader(program, vertex_shader_object);
	glAttachShader(program, fragment_shader_object);

	glBindAttribLocation(program, Vertex_Position, "vertex_position");
	glBindAttribLocation(program, Vertex_Texture_Coordinate, "vertex_texture_coordinate");
	glBindAttribLocation(program, Vertex_Color, "vertex_color");

	glLinkProgram(program);

	s32 is_linked;
	glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
	assert(is_linked);


	return program;
}

void ReloadRenderContext(render_context *renderer) {
	GlLoad(glGenBuffers);
	GlLoad(glBindBuffer);
	GlLoad(glDeleteBuffers);
	GlLoad(glBufferData);
	GlLoad(glVertexAttribPointer);
	GlLoad(glEnableVertexAttribArray);
	GlLoad(glDisableVertexAttribArray);
//	GlLoad(glTexCoordPointer);
//	GlLoad(glColorPointer);
	GlLoad(glGenVertexArrays);
	GlLoad(glBindVertexArray);
	GlLoad(glDeleteVertexArrays);
//	GlLoad(glDrawArrays);
	GlLoad(glDebugMessageCallback);
	GlLoad(glCreateProgram);
	GlLoad(glAttachShader);
	GlLoad(glCreateShader);
	GlLoad(glDeleteProgram);
	GlLoad(glLinkProgram);
	GlLoad(glDeleteShader);
	GlLoad(glUseProgram);
	GlLoad(glBindAttribLocation);
	GlLoad(glUniform1i);
	GlLoad(glShaderSource);
	GlLoad(glCompileShader);
	GlLoad(glGetShaderiv);
	GlLoad(glGetProgramiv);
	GlLoad(glGetUniformLocation);
	GlLoad(glActiveTexture);
}

render_context MakeRenderContext(f32 target_width_over_height, f32 canvas_width_in_texels) {
	ReloadRenderContext(null);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlHandleDebugMessage, null);

	char vertex_source[] = 
R"(
#version 330
in vec3 vertex_position;
in vec2 vertex_texture_coordinate;
in vec4 vertex_color;

out vec2 fragment_texture_coordinate;
out vec4 fragment_color;

void main() {
	fragment_texture_coordinate = vertex_texture_coordinate;
	fragment_color = vertex_color;
	gl_Position = vec4(vertex_position, 1);
}
)";
	
	char fragment_source[] = 
R"(
#version 330
in vec2 fragment_texture_coordinate;
in vec4 fragment_color;

uniform sampler2D diffuse_texture;

out vec4 pixel_color;

void main() {
	vec4 diffuse_color = texture(diffuse_texture, fragment_texture_coordinate);
	pixel_color = fragment_color * diffuse_color;
}
)";
	
	u32 vertex_shader = MakeShaderObject(GL_VERTEX_SHADER, vertex_source);
	u32 fragment_shader = MakeShaderObject(GL_FRAGMENT_SHADER, fragment_source);
	render_context context = {};
	context.program = MakeProgramObject(vertex_shader, fragment_shader);

	context.diffuse_texture_uniform = glGetUniformLocation(context.program, "diffuse_texture");

	rgba32 white = Color32(1.0f, 1.0f, 1.0f);
	context.white_texture = MakeTexture(1, 1, u8_array{white.values, 4});

	context.target_width_over_height = target_width_over_height;
	context.canvas_width_in_texels = canvas_width_in_texels;
	context.canvas_height_in_texels = canvas_width_in_texels * target_width_over_height;
	context.canvas_texel_width = 1.0f / canvas_width_in_texels;
	context.canvas_texel_height = context.canvas_texel_width * target_width_over_height;

	return context;
}

f32 Lerp(f32 a, f32 b, f32 blend_factor) {
	f32 result = a * (1 - blend_factor) + b * blend_factor;
	return result;
}

rgba32 Color32(f32 r, f32 g, f32 b, f32 a) {
	return {(u8) (r * 255), (u8) (g * 255), (u8) (b * 255), (u8) (a * 255)};
}

texture MakeTexture (s32 width, s32 height, u8_array pixels) {
	texture result;
	
	glGenTextures(1, &result.id);
	glBindTexture(GL_TEXTURE_2D, result.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.base);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
	result.size.width = width;
	result.size.height = height;
    
	return result;
}

void DrawTexturedRect(render_context *renderer, texture my_texture, vec2s position, box2s texture_box, f32 x_alignment, f32 y_alignment, f32 x_flip, f32 y_flip, rgba32 color, f32 z) {
    auto command = renderer->commands + renderer->command_count; 
    renderer->command_count++;
    assert(renderer->command_count <= ArrayCountOf(renderer->commands));
    
    command->texture_id = my_texture.id;
    command->vertex_offset = renderer->vertex_count;
    command->vertex_count = 6;
    
    f32 texel_width = 1.0f / my_texture.size.width; 
    f32 texel_height = 1.0f / my_texture.size.height;
    
    f32 canvas_texel_width = renderer->canvas_texel_width;
    f32 canvas_texel_height = renderer->canvas_texel_height;
    x_alignment = Lerp(x_alignment, 1 - x_alignment, x_flip);
    y_alignment = Lerp(y_alignment, 1 - y_alignment, y_flip);
    
    auto size = texture_box.max - texture_box.min;
    
    position.x -= x_alignment * size.width;
    position.y -= y_alignment * size.height;
    
    f32 tx_l = texture_box.min.x * texel_width;
    f32 tx_r = texture_box.max.x * texel_width;
    f32 tx0 = Lerp(tx_l, tx_r, x_flip);
    f32 tx1 = Lerp(tx_r, tx_l, x_flip);
    
    f32 ty_l = texture_box.min.y * texel_height;
    f32 ty_r = texture_box.max.y * texel_height;
    f32 ty0 = Lerp(ty_l, ty_r, y_flip);
    f32 ty1 = Lerp(ty_r, ty_l, y_flip);
    
    auto vertices = renderer->vertices + renderer->vertex_count;
    renderer->vertex_count += 6;
    assert(renderer->vertex_count <= ArrayCountOf(renderer->vertices));
    
    vertices[0].x = position.x * canvas_texel_width;
    vertices[0].y = position.y * canvas_texel_height;
    vertices[0].z = z;
    vertices[0].u = tx0;
    vertices[0].v = ty0;
    vertices[0].color = color;
    
    vertices[1].x = (position.x + size.width) * canvas_texel_width;
    vertices[1].y = position.y * canvas_texel_height;
    vertices[1].z = z;
    vertices[1].u = tx1;
    vertices[1].v = ty0;
    vertices[1].color = color;
    
    vertices[2].x = (position.x + size.width) * canvas_texel_width;
    vertices[2].y = (position.y + size.height) * canvas_texel_height;
    vertices[2].z = z;
    vertices[2].u = tx1;
    vertices[2].v = ty1;
    vertices[2].color = color;
    
    vertices[3].x = position.x * canvas_texel_width;
    vertices[3].y = (position.y + size.height) * canvas_texel_height;
    vertices[3].z = z;
    vertices[3].u = tx0;
    vertices[3].v = ty1;
    vertices[3].color = color;
    
    vertices[4] = vertices[0];
    vertices[5] = vertices[2];
}

void DrawRect(render_context *renderer, vec2s position, s32 width, s32 height, rgba32 color, f32 x_alignment, f32 y_alignment, f32 z) {
    DrawTexturedRect(renderer, renderer->white_texture, position, { 0, 0, width, height }, x_alignment, y_alignment, 0, 0, color, z);
}

void DrawTexture(render_context *renderer, texture my_texture, vec2s position, f32 x_alignment, f32 y_alignment, f32 x_flip, f32 y_flip, rgba32 color, f32 z) {
    	DrawTexturedRect(renderer, my_texture, position, { 0, 0, my_texture.size.width, my_texture.size.height }, x_alignment, y_alignment, x_flip, y_flip, color, z);
}

void Render(render_context *renderer) {
    u32 vertex_buffer, vertex_array;

    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, renderer->vertex_count * sizeof(render_vertex), renderer->vertices, GL_STREAM_DRAW);

#if 1
	glEnableVertexAttribArray(Vertex_Position);
	glEnableVertexAttribArray(Vertex_Texture_Coordinate);
	glEnableVertexAttribArray(Vertex_Color);

	glVertexAttribPointer(Vertex_Position, 3, GL_FLOAT, GL_FALSE,sizeof(render_vertex), (void *) 0);
	glVertexAttribPointer(Vertex_Texture_Coordinate, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *) (3 * sizeof(f32)));
	glVertexAttribPointer(Vertex_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(render_vertex), (void *) (5 * sizeof(f32)));
#else
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT,sizeof(render_vertex), (void *) 0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(render_vertex), (void *) (3 * sizeof(f32)));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(render_vertex), (void *) (5 * sizeof(f32)));
#endif

    glDisable(GL_DEPTH_TEST);

    glUseProgram(renderer->program);
    glUniform1i(renderer->diffuse_texture_uniform, 0);
    glActiveTexture(GL_TEXTURE0 + 0);

    for (u32 i = 0; i < renderer->command_count; ++i) {
        glBindTexture(GL_TEXTURE_2D, renderer->commands[i].texture_id);
        glDrawArrays(GL_TRIANGLES, renderer->commands[i].vertex_offset, renderer->commands[i].vertex_count);
    }

    glDeleteBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    renderer->vertex_count = 0;
    renderer->command_count = 0;
}