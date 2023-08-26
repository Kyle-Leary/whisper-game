#include "static.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "im_prims.h"
#include "objects/camera.h"

// a nearly blank level with a camera about the center for testing.

static vec3 camera_focus;

// setup all the local objects in the scene.
void areas_static() {
  glm_vec3_zero(camera_focus);

  // note that the camera is directly LINKED to the our_player's lerp_position
  // from its physics representation.
  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);
}

void areas_static_update() {
  for (int i = 0; i < 20; i++) {
    float x = i * 0.1f;
    float y = i * 0.2f;
    float z = 0.0f;
    im_point((vec3){x, y, z});
  }
}
