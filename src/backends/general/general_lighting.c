// define lighting helper functions for the graphics api that just manipulate
// the global structure.

#include "backends/graphics_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// might as well zero-alloc, since we want the counts to init to zero.
// by default, we aren't using any slots.
LightData g_light_data = {0};

// copy it into the new slot.
LightSlot g_add_point_light(PointLight *light) {
  // slot is an index, n_point_lights is a length.
  LightSlot this_slot = g_light_data.n_point_lights;
  if (this_slot >= POINT_LIGHT_SLOTS) {
    fprintf(stderr,
            "ERROR: Too many point lights! Cannot add the point light %p.\n",
            light);
    exit(1);
  } else {
    memcpy(&g_light_data.point_lights[this_slot], light, sizeof(PointLight));
    g_light_data.n_point_lights++;
  }
  return this_slot;
}

// bump down the number and potentially reorganize the array.
void g_remove_point_light(LightSlot slot) {
  if (slot >= POINT_LIGHT_SLOTS) {
    fprintf(stderr,
            "ERROR: point light removal out-of-bounds. Tried to remove "
            "light slot %d.\n",
            slot);
    exit(1);
  }

  LightSlot last_slot = g_light_data.n_point_lights - 1;
  if (slot == last_slot) {
    // we're removing the last element of the array.
    // we don't have to reorganize the array, so do nothing?
  } else {
    // we have to reorganize.
    for (int i = slot + 1; i < g_light_data.n_point_lights; i++) {
      // copy from right to left backward for each element after the break in
      // the middle of the list we've created.
      g_light_data.point_lights[i - 1] = g_light_data.point_lights[i];
    }
  }
  g_light_data.n_point_lights--;
}
