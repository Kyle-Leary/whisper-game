#pragma once

#include "cglm/types.h"
#include <stdint.h>

// fill the indices array with all the things the raycast collides at, and stop
// at num_indices. indices after calling is full of indices into the global
// object_state objects array.
//
// the return value is the number of indices we actually found. if we reach the
// caller's limit early, we return out of the function.
int raycast_intersect(uint16_t *indices, int num_indices, vec3 origin,
                      vec3 direction);
