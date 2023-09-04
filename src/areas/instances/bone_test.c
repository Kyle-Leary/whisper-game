#include "cglm/types.h"
#include "math/mat.h"
#include "objects/camera.h"
#include "path.h"
#include "render/light.h"
#include "render/model.h"
#include "render/render.h"

static vec3 camera_focus;

void areas_bone_test() {
  glm_vec3_zero(camera_focus);

  RenderComp *r = make_rendercomp_from_glb(MODEL_PATH("rouge_rig.glb"));
  Model *m = (Model *)r->data;

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

void areas_bone_test_update() {}
