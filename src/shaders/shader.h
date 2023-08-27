#pragma once

// the actual internal logic on each shader type. the instance logic goes in
// shader_instances.{c,h}

#include "shaders/shader_instances.h"
#include <stdbool.h>

// return the shader id, not the Shader* structure.
int make_shader(const char *shader_path);

void shader_general_setup(Shader *s);

void shader_use(Shader *program);
void shader_use_name(const char *name);

void shader_set_1f(Shader *shader, const char *uniform_name, float f0);
void shader_set_2f(Shader *shader, const char *uniform_name, float f0,
                   float f1);
void shader_set_3f(Shader *shader, const char *uniform_name, float f0, float f1,
                   float f2);
void shader_set_4f(Shader *shader, const char *uniform_name, float f0, float f1,
                   float f2, float f3);

void shader_set_1i(Shader *shader, const char *uniform_name, int i0);
void shader_set_2i(Shader *shader, const char *uniform_name, int i0, int i1);
void shader_set_3i(Shader *shader, const char *uniform_name, int i0, int i1,
                   int i2);
void shader_set_4i(Shader *shader, const char *uniform_name, int i0, int i1,
                   int i2, int i3);

void shader_set_matrix2fv(Shader *shader, const char *uniform_name,
                          const float *value);
void shader_set_matrix3fv(Shader *shader, const char *uniform_name,
                          const float *value);
void shader_set_matrix4fv(Shader *shader, const char *uniform_name,
                          const float *value);
