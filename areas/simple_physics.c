// physics testing and demos of simple primitive collision.

#include "cglm/types.h"
#include "math/mat.h"
#include "objects/camera.h"
#include "objects/floor.h"
#include "objects/sphere.h"
#include "path.h"
#include "render/light.h"
#include "render/model.h"
#include "render/render.h"

static vec3 camera_focus;

void init() {
  glm_vec3_zero(camera_focus);

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);

  object_add((Object *)sphere_build((vec3){0.1, 12, 0.9}, 2.5f, 10), OT_AREA);
  object_add((Object *)sphere_build((vec3){0, 9, 0}, 1.0f, 10), OT_AREA);
  object_add((Object *)sphere_build((vec3){0, 2, 0}, 1.0f, 10), OT_AREA);
  object_add((Object *)floor_build((vec3){0}), OT_AREA);

  {   // setup global lights
    { // setup ambient light
      g_light_data.ambient_light.color[0] = 0.1f;
      g_light_data.ambient_light.color[1] = 0.1f;
      g_light_data.ambient_light.color[2] = 0.1f;
      g_light_data.ambient_light.color[3] = 1.0f;
      g_light_data.ambient_light.intensity = 0.5f;
    }

    {
      PointLight pl;

      pl.intensity = 50.5f;
      pl.position[0] = 0.0f;
      pl.position[1] = 15.0f;
      pl.position[2] = 0.0f;
      pl.color[0] = 0.5f;
      pl.color[1] = 0.9f;
      pl.color[2] = 0.5f;
      pl.color[3] = 1.0f;
      pl.falloff = 0.5f;

      w_ca_add_PointLight(&g_light_data.point_light_ca, &pl);
    }
  }
}

void update() {}

void clean() {}
