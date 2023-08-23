#include "cube.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "physics/collider_types.h"
#include "render.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Cube entity type.

#define CAST Cube *cube = (Cube *)p

Cube *cube_build(vec3 position) {
  Cube *p = (Cube *)malloc(sizeof(Cube));

  p->type = OBJ_CUBE;

  {
    Collider *colliders = NULL;
    p->phys = make_physcomp(0.1, 1.0, 0.5, 0.5, 0.3, true, false, colliders, 1,
                            position);
  }

  p->render =
      make_rendercomp_from_graphicsrender(glprim_cube(p->phys->position));

  return p;
}

void cube_init(void *p) {}

void cube_update(void *p) {}

void cube_handle_collision(void *p, CollisionEvent *e) {
  Cube *cube = (Cube *)p;
  glm_vec3_scale(e->normalized_force, e->magnitude, e->normalized_force);
  glm_vec3_add(cube->phys->position, e->normalized_force, cube->phys->position);
}

void cube_clean(void *p) {
  Cube *cube = (Cube *)p;
  free(cube);
}
