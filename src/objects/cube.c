#include "cube.h"

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
// really basic Cube entity type.

#define CAST Cube *cube = (Cube *)p

Cube *cube_build(vec3 position) {
  Cube *p = (Cube *)malloc(sizeof(Cube));
  memcpy(p->position, position, sizeof(float) * 3);
  p->colliders =
      (Collider *)malloc(sizeof(Collider) * 1); // space for one collider.
  p->colliders[0].type = CL_PILLAR;
  p->type = OBJ_CUBE;
  p->num_colliders = 0;
  p->render = glprim_cube(p->position);
  return p;
}

void cube_init(void *p) {}

void cube_update(void *p) {}

void cube_draw(void *p) {
  CAST;
  g_draw_render(cube->render);
}

void cube_handle_collision(void *p, CollisionEvent *e) {
  Cube *cube = (Cube *)p;
  glm_vec3_scale(e->normalized_force, e->magnitude, e->normalized_force);
  glm_vec3_add(cube->position, e->normalized_force, cube->position);
}

void cube_clean(void *p) {
  Cube *cube = (Cube *)p;
  free(cube);
}
