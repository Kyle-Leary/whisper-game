#include "cube.h"

#include "../object.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"

#include "physics/body/body.h"
#include "physics/collider/collider.h"
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
    p->phys = make_physcomp(
        (Body *)make_rigid_body(0.1, 1.0, 0.5, 0.5, 0.3, true, position),
        (Collider *)make_rect_collider());
  }

  p->render = make_rendercomp_from_graphicsrender(
      glprim_cube(((RigidBody *)p->phys->body)->position));

  return p;
}

void cube_init(void *p) {}

void cube_update(void *p) {}

void cube_clean(void *p) {
  Cube *cube = (Cube *)p;
  free(cube);
}
