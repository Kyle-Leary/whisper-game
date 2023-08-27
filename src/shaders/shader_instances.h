#pragma once

#include "cglm/types.h"
#include "whisper.h"
#include "whisper/colmap.h"

#define FONT_TEX_SLOT 7
#define SKYBOX_TEX_SLOT 8

typedef struct Shader Shader;

// apply a cache on top of the hashmap to store the locations, grabbing them
// lazily when we need them.
typedef struct Shader {
  int id; // the whole program id, only complete programs have uniform id
          // getters in opengl, not individual vert/frag/geo shaders etc.
  WColMap locs;

  void (*bind)(Shader *);
  void (*unbind)(Shader *);
  // allow the shader to handle the model polymorphically. if this is NULL, the
  // program will just set u_model to the passed in model by default.
  void (*handle_model)(Shader *, mat4);

  char name[32];
} Shader;

// throw a loud error if we try to get a shader before it's added.
// the shader map should be fairly static, so it should be safe to trust
// references returned by this function. it just grabs a pointer by a key in an
// internal hashmap stored by the instantiation module.
Shader *get_shader(const char *key);

// this also acts as a kind of "pipeline init" function, alongside creating all
// the shaders in the map.
void shader_instantiate_all();

void shader_destroy_all();
