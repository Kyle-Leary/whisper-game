#pragma once

#include "../ogl_includes.h"

typedef struct Shader {
  GLuint id;
} Shader;

GLuint make_shader(const char *vs_path, const char *fs_path);
