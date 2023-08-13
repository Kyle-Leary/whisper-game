#pragma once

#include "animation/anim_struct.h"
#include "backends/graphics_api.h"
typedef enum Properties {
  P_INVALID = 0,
  P_TRANSLATION,
  P_ROTATION,
  P_SCALE,
  P_UNKNOWN,
} Properties;

Properties prop_from_string(const char *prop_string);

// return the base ptr of the data "pointed" to by the Target structure in the
// glb hierarchy.
void *return_prop_base_ptr(Node *nodes, Target *target, int *prop_sz);
