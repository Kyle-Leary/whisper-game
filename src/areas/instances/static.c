#include "cglm/types.h"
#include "cglm/vec3.h"
#include "gui/gui.h"
#include "gui/gui_layouts.h"
#include "gui/gui_prim.h"
#include "gui/widgets.h"
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

void function_one() { printf("function_one\n"); }

void function_two() { printf("function_two\n"); }

void areas_static_update() {
  im_cube((vec3){0}, 1);
  im_cube((vec3){0, 8, 0}, 1);

  for (int i = 0; i < 9; i++) {
    im_point((vec3){i + 1, i, 0});
    im_point((vec3){0, i, 0});
  }

  gui_push((Layout *)&(LayoutVertical){
      .type = LAYOUT_VERTICAL, .margin = 0.01, .padding = 0.09});
  gui_draggable("v");

  gui_draggable("h");
  gui_draggable("k");

  gui_push(NULL);
  gui_draggable("w");

  GUIFunctionListInput input = {
      .num_inputs = 5,
      .inputs =
          {
              {function_one, "function one"},
              {function_two, "function two"},
              {function_two, "function twoo"},
              {function_two, "function twooo"},
              {function_two, "function twoooo"},
          },
  };
  gui_function_list(&input);

  gui_pop();
  gui_pop();

  // {
  //   gui_push(NULL);
  //   gui_draggable("subwindow");
  //   gui_label("top left", "top left");
  //   gui_label("top right", "top right");
  //   gui_label("bottom left", "bottom left");
  //   gui_label("bottom right", "bottom right");
  //   gui_pop();
  // }
}
