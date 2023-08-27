#pragma once

#include "render/graphics_render.h"
#include "render/model.h"

// functions that print stuff.

void print_int(int i, int indent_level);
void print_vec2(vec2 v, int indent_level);
void print_vec3(vec3 v, int indent_level);
void print_vec4(vec4 v, int indent_level);
void print_many_int(int *ints, int num_ints, int indent_level);
void print_many_uint(unsigned int *uints, int num_uints, int indent_level);
void print_many_vec2(float *vec2s, int num_vec2s, int indent_level);
void print_many_vec3(float *vec3s, int num_vec3s, int indent_level);
void print_many_vec4(float *vec4s, int num_vec4s, int indent_level);
void print_mat4(mat4 m, int indent_level);

void print_model(const Model *model, int indent_offset);
void print_graphics_render(const GraphicsRender *gr, int indent_offset);
