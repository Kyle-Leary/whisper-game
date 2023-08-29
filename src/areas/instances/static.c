#include "cglm/types.h"
#include "cglm/vec3.h"
#include "gui/gui.h"
#include "im_prims.h"
#include "immediate.h"
#include "objects/camera.h"

// a nearly blank level with a camera about the center for testing.

static vec3 camera_focus;

// setup all the local objects in the scene.
void areas_static() {
  glm_vec3_zero(camera_focus);

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);
}

void areas_static_update() {
  // for (int i = 0; i < 10; i++) {
  //   float positions[9] = {10, 10, 10, 10, -10, -10, -10, -10, -10};
  //   im_draw((float *)positions, 3, (vec4){1, 1, 1, 1}, IM_TRIANGLES);
  // }

  // gui_draggable("subwindow", &(AABB){0.5, 0.5, 0.5, 0.5});

  {
    gui_push();
    gui_draggable("subwindow", &(AABB){0.5, 0.5, 0.25, 0.5});

    gui_label("top left", "top left", &(AABB){0.25, 0.75, 0.24, 0.24});
    gui_label("top right", "top right", &(AABB){0.75, 0.75, 0.24, 0.24});

    gui_label("bottom left", "bottom left", &(AABB){0.25, 0.25, 0.24, 0.24});
    gui_label("bottom right", "bottom right", &(AABB){0.75, 0.25, 0.24, 0.24});

    gui_pop();
  }
}

// comment
// comment
