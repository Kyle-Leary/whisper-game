#pragma once

#include "whisper.h"
#include "whisper/colmap.h"

// apply a cache on top of the hashmap to store the locations, grabbing them
// lazily when we need them.
typedef struct Shader {
  int id; // the whole program id, only complete programs have uniform id
          // getters in opengl, not individual vert/frag/geo shaders etc.
  WHashMap locs;

  void (*bind)();
  void (*unbind)();
} Shader;

void shader_instantiate_all();

void shader_destroy_all();
