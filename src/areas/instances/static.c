#include "cglm/types.h"
#include "cglm/vec3.h"
#include "gui/gui.h"
#include "im_prims.h"
#include "immediate.h"
#include "objects/camera.h"
#include "render/gr_prim.h"
#include "render/render.h"

// a nearly blank level with a camera about the center for testing.

static vec3 camera_focus;

// setup all the local objects in the scene.
void areas_static() {
  glm_vec3_zero(camera_focus);

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);
}

void areas_static_update() {
  im_cube((vec3){0}, 1);
  im_cube((vec3){0, 8, 0}, 1);

  for (int i = 0; i < 9; i++) {
    im_point((vec3){i + 1, i, 0});
    im_point((vec3){0, i, 0});
  }

  gui_draggable("some text", &(AABB){0.1, 0.1, 0.1, 0.1});

  {
    gui_push();
    gui_draggable("subwindow", &(AABB){0.1, 0.2, 0.1, 0.1});
    gui_label("top left", "top left", &(AABB){0.25, 0.75, 0.24, 0.24});
    gui_label("top right", "top right", &(AABB){0.75, 0.75, 0.24, 0.24});
    gui_label("bottom left", "bottom left", &(AABB){0.25, 0.25, 0.24, 0.24});
    gui_label("bottom right", "bottom right", &(AABB){0.75, 0.25, 0.24, 0.24});
    gui_pop();
  }
}
