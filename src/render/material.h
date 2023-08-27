#pragma once

// PBR material structure, this is similarly slotted directly into the block UBO
// data.
#include "cglm/types.h"
#include <stdbool.h>

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
typedef struct ModelMaterial {
  MaterialData inner;

  bool double_sided;
} ModelMaterial;

// all renders from now on will use the mat Material.
// all shaders are implicitly PBR shaders?
void g_use_material(MaterialData *mat);
