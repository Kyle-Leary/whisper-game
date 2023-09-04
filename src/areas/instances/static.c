#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "global.h"
#include "gui/gui.h"
#include "gui/gui_layouts.h"
#include "gui/gui_prim.h"
#include "gui/widgets.h"
#include "helper_math.h"
#include "im_prims.h"
#include "immediate.h"
#include "math/mat.h"
#include "objects/camera.h"
#include "path.h"
#include "render/gr_prim.h"
#include "render/light.h"
#include "render/model.h"
#include "render/render.h"

// a nearly blank level with a camera about the center for testing.

static vec3 camera_focus;

static mat4 model;

// setup all the local objects in the scene.
void areas_static() {
  glm_vec3_zero(camera_focus);

  // RenderComp *r = make_rendercomp_from_glb(MODEL_PATH("tall_rig.glb"));
  // Model *m = (Model *)r->data;

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);

  {   // setup global lights
    { // setup ambient light
      g_light_data.ambient_light.color[0] = 1.0f;
      g_light_data.ambient_light.color[1] = 0.5f;
      g_light_data.ambient_light.color[2] = 0.5f;
      g_light_data.ambient_light.color[3] = 1.0f;
      g_light_data.ambient_light.intensity = 0.5f;
    }

    {
      PointLight pl;

      pl.intensity = 1.0f;
      pl.position[0] = 0.0f;
      pl.position[1] = 0.0f;
      pl.position[2] = 0.0f;
      pl.color[0] = 0.5f;
      pl.color[1] = 0.1f;
      pl.color[2] = 0.5f;
      pl.color[3] = 1.0f;

      w_ca_add_PointLight(&g_light_data.point_light_ca, &pl);
    }
  }
}

void function_one() { printf("function_one\n"); }

void function_two() { printf("function_two\n"); }

void areas_static_update() {

  // glm_rotate(model, glm_rad(1), (vec3){0, 1, 0});
  // glm_translate(model, (vec3){0, 0.01, 0});

  float t = u_time - floorf(u_time);

  glm_mat4_identity(model);
  mat4 shear = {{1, bounce_interp(0, 1, t), 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1}};
  mat4_mul(shear, model, model);

  im_identity_grid(2, 0.2);
  im_grid(model, 2, 0.5);

  // gui_push((Layout *)&(LayoutVertical){
  //     .type = LAYOUT_VERTICAL, .margin = 0.01, .padding = 0.09});
  // gui_draggable("v");
  //
  // gui_draggable("h");
  // gui_draggable("k");
  //
  // gui_push(NULL);
  // gui_draggable("w");
  //
  // GUIFunctionListInput input = {
  //     .num_inputs = 5,
  //     .inputs =
  //         {
  //             {function_one, "function one"},
  //             {function_two, "function two"},
  //             {function_two, "function twoo"},
  //             {function_two, "function twooo"},
  //             {function_two, "function twoooo"},
  //         },
  // };
  // gui_function_list(&input);
  //
  // gui_pop();
  // gui_pop();
  //
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
