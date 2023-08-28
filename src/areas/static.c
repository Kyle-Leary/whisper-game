#include "static.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "gui/gui.h"
#include "im_prims.h"
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
  if (gui_button("helloworldlabel", "helloworld",
                 &(AABB){0.4, 0.2, 0.05, 0.02})) {
    printf("hello\n");
  }

  if (gui_button("sldkfjlaskjdfhelloworldlabel", "helloworld",
                 &(AABB){0.4, 0.8, 0.05, 0.02})) {
    printf("hello\n");
  }

  gui_label("blah", "laksjdfhello", &(AABB){0.5, 0.8, 0.1, 0.1});
  gui_draggable("draggablestuff", &(AABB){0.3, 0.3, 0.2, 0.2});
}
