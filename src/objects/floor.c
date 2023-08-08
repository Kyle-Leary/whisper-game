#include "floor.h"

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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Floor entity type.

#define CAST Floor *floor = (Floor *)p

Floor *floor_build(vec3 position, float strength) {
  Floor *p = (Floor *)calloc(sizeof(Floor), 1);
  p->type = OBJ_FLOOR;

  memcpy(p->position, position, sizeof(float) * 3);

  p->num_colliders = 1;
  p->colliders = (Collider *)malloc(sizeof(Collider) * p->num_colliders);
  p->colliders[0].type = CL_FLOOR;

  FloorColliderData *col_data =
      (FloorColliderData *)malloc(sizeof(FloorColliderData) * 1);
  col_data->strength = strength;
  p->colliders[0].data = col_data;

  p->immovable = 1;

  p->render = glprim_cube(p->position);
  return p;
}

void floor_init(void *p) {}

void floor_update(void *p) {}

void floor_draw(void *p) {
  CAST;
  g_draw_render(floor->render);
}

void floor_handle_collision(void *p, CollisionEvent *e) {}

void floor_clean(void *p) {
  Floor *floor = (Floor *)p;
  free(floor);
}
