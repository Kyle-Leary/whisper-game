#pragma once

// management, lifecycle and modification for UBOs.

// ubo block number defs.
typedef enum UBOBlock {
  LIGHT_BLOCK,
  MATRIX_BLOCK,
  BONE_BLOCK,
  MATERIAL_BLOCK,
  BLOCK_COUNT,
} UBOBlock;

void ubo_init();
void ubo_update();
