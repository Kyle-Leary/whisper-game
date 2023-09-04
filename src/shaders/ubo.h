#pragma once

#include "render/material.h"
#include "render/rigging.h"

// management, lifecycle and modification for UBOs.

// ubo block number defs.

typedef enum UBOBlock {
  LIGHT_BLOCK,
  MATRIX_BLOCK,
  BONE_BLOCK,
  MATERIAL_BLOCK,
  BLOCK_COUNT,
} UBOBlock;

// create the ubo buffers, stick them in the static array.
void ubo_init();

// some of the ubos are just updated in a loop every frame.
void ubo_update();

// some need to be manually pushed at certain points in the rendering pipeline.
void ubo_push_material(MaterialData *mat);
void ubo_push_bones(BoneData *bones);
