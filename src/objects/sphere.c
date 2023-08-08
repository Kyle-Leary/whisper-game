#include "sphere.h"

#include "cglm/types.h"
#include "cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Sphere entity type.

#define CAST Sphere *sphere = (Sphere *)p

Sphere *sphere_build(vec3 position, float radius, unsigned int segments) {
  Sphere *p = (Sphere *)calloc(sizeof(Sphere), 1);
  p->type = OBJ_SPHERE;

  memcpy(p->position, position, sizeof(float) * 3);
  memcpy(p->lerp_position, p->position, sizeof(float) * 3);

  p->num_colliders = 1;
  p->colliders = (Collider *)malloc(sizeof(Collider) * p->num_colliders);
  p->colliders[0].type = CL_SPHERE;

  p->position_lerp_speed = 0.9F;
  p->mass = 0.1;
  p->linear_damping = 0.5;

  SphereColliderData *col_data =
      (SphereColliderData *)malloc(sizeof(SphereColliderData) * 1);
  col_data->radius = radius;
  p->colliders[0].data = col_data;

  p->render = glprim_sphere(p->position, radius, segments);
  return p;
}

void sphere_init(void *p) {}

void sphere_update(void *p) { CAST; }

void sphere_draw(void *p) {
  CAST;
  glm_mat4_identity(sphere->render->model);
  glm_translate(sphere->render->model, sphere->lerp_position);
  g_draw_render(sphere->render);
}

void sphere_handle_collision(void *p, CollisionEvent *e) {}

void sphere_clean(void *p) {
  Sphere *sphere = (Sphere *)p;
  free(sphere);
}
