#pragma once

// PBR material structure, this is similarly slotted directly into the block UBO
// data.
#include "cglm/types.h"
#include <stdbool.h>
#include <sys/types.h>

typedef struct MaterialData {
  // goes by the PBR spec generally laid out by the glb format.
  vec4 albedo;
  float metallic;
  char padding1[12];
  float roughness;
  char padding2[12];
  vec3 emissive_factor;
  char padding8[4];
  bool double_sided;
  char padding9[12];
} MaterialData;

// have a wrapper structure that can double as both the raw UBO data and
// something more expressive, without having to pointer-chase to pull the
// structure back together.
typedef struct Material {
  MaterialData inner;
  // if a texture has an id of 0, it'll simply unbind the current texture. this
  // is fine.
  uint base_color_texture;
  bool double_sided;
} Material;

// enable the texture in the pipeline for the preceding draw call.
void g_use_material(Material *mat);
void free_material(Material *mat);
