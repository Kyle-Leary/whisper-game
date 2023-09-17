#include "animation/animator.h"
#include "cglm/types.h"
#include "input/input.h"
#include "math/mat.h"
#include "objects/camera.h"
#include "path.h"
#include "render/light.h"
#include "render/model.h"
#include "render/render.h"

#include "macros.h"

static vec3 camera_focus;

static Animator *a;
static int i;

void init() {
  glm_vec3_zero(camera_focus);

  RenderComp *r = make_rendercomp_from_glb(MODEL_PATH("long.glb"));
  Model *m = (Model *)r->data.model;
  a = make_animator(m);
  // anim_play(a, "wiggle", true);
  i = 0;

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

void update() {
  if (i_state.act_just_pressed[ACT_UP]) {
    INFO("hello");
    anim_force_tick(a, i);
  }
}
