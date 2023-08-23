#include "floor.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/affine-pre.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "printers.h"
#include "render.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAST Floor *floor = (Floor *)p

Floor *floor_build(vec3 position, float strength) {
  Floor *p = (Floor *)calloc(sizeof(Floor), 1);
  p->type = OBJ_FLOOR;

  {
    Collider *colliders = (Collider *)calloc(sizeof(Collider), 1);
    make_colliders(1, colliders);

    colliders[0].type = CL_FLOOR;
    FloorColliderData *col_data =
        (FloorColliderData *)malloc(sizeof(FloorColliderData) * 1);
    col_data->strength = strength;
    colliders[0].data = col_data;

    p->phys = make_physcomp(0.1, 1.0, 0.5, 0.5, 0.3, false, true, colliders, 1,
                            position);
  }

  p->render = make_rendercomp_from_graphicsrender(
      glprim_floor_plane(p->phys->lerp_position));
  GraphicsRender *prim = (GraphicsRender *)p->render->data;
  glm_scale(prim->model, (vec3){100, 1, 100});
  glm_translate(prim->model, position);
  return p;
}

void floor_init(void *p) {}

void floor_update(void *p) {}

void floor_handle_collision(void *p, CollisionEvent *e) {}

void floor_clean(void *p) {
  Floor *floor = (Floor *)p;
  free(floor);
}
