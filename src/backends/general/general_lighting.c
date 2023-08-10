// define lighting helper functions for the graphics api that just manipulate
// the global structure.

#include "backends/graphics_api.h"
#include <stdio.h>
#include <string.h>

// might as well zero-alloc, since we want the counts to init to zero.
// by default, we aren't using any slots.
LightData g_light_data = {0};

// copy it into the new slot.
LightSlot g_add_point_light(PointLight *light) {
  LightSlot this_slot = g_light_data.n_point_lights;
  if (this_slot >= POINT_LIGHT_SLOTS) {
    fprintf(stderr,
            "ERROR: Too many point lights! Cannot add the point light %p.\n",
            light);
  } else {
    memcpy(&g_light_data.point_lights[this_slot], light, sizeof(PointLight));
    g_light_data.n_point_lights++;
  }
  return this_slot;
}

// bump down the number and optionally reorganize the array.
void g_remove_point_light(LightSlot slot) {
  LightSlot last_slot = g_light_data.n_point_lights - 1;
  if (slot == last_slot) {
    // we don't have to reorganize the array, so do nothing?
  } else {
    // we have to reorganize.
  }
  g_light_data.n_point_lights--;
}
