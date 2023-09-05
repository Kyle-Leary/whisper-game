#include "animation/anim_struct.h"
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

  mat4_identity(model);
}

typedef enum AnimationSetting {
  AS_GROW,
  AS_SHRINK,
  AS_IDENTITY,
  AS_SHEAR,
  AS_COUNT,
} AnimationSetting;

static AnimationSetting setting = AS_GROW;

void inc_anim() {
  mat4_identity(model);
  setting++;
  setting %= AS_COUNT;
}

void areas_static_update() {
  float t = u_time - floorf(u_time);

  int grid_sz = 2;

  im_identity_grid(grid_sz, 0.2);

  switch (setting) {
  case AS_GROW: {
    mat4_scale(model, 1.001, model);
    im_grid(model, grid_sz, 0.5);
  } break;

  case AS_SHRINK: {
    mat4_scale(model, 0.999, model);
    im_grid(model, grid_sz, 0.5);
  } break;

  case AS_IDENTITY: {
    glm_mat4_identity(model);
    im_grid(model, grid_sz, 0.5);
  } break;

  case AS_SHEAR: {
    glm_mat4_identity(model);
    mat4 shear = {{1, bounce_interp(0, 1, t), 0, 0},
                  {0, 1, 0, 0},
                  {0, 0, 1, 0},
                  {0, 0, 0, 1}};
    mat4_mul(shear, model, model);
    im_grid(model, grid_sz, 0.5);
  } break;

  default: {
  } break;
  }

  gui_push((Layout *)&(LayoutVertical){
      .type = LAYOUT_VERTICAL, .margin = 0.01, .padding = 0.09});
  gui_widget("v");

  gui_widget("h");
  gui_widget("k");
  gui_widget("a");
  gui_widget("b");
  gui_widget("c");

  gui_push(NULL);
  gui_draggable("w");

  GUIFunctionListInput input = {
      .num_inputs = 1,
      .inputs =
          {
              {inc_anim, "next anim"},
          },
  };
  gui_function_list(&input);
}
