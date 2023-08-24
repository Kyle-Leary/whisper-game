#include "floor.h"

#include "../object.h"
#include "backends/graphics_api.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "physics/body/body.h"
#include "physics/collider/collider.h"
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
    p->phys = make_physcomp((Body *)make_static_body(position),
                            (Collider *)make_floor_collider());
  }

  StaticBody *sb = (StaticBody *)p->phys->body;

  p->render =
      make_rendercomp_from_graphicsrender(glprim_floor_plane(sb->position));
  GraphicsRender *prim = (GraphicsRender *)p->render->data;
  glm_scale(prim->model, (vec3){100, 1, 100});
  glm_translate(prim->model, position);
  return p;
}

void floor_init(void *p) {}

void floor_update(void *p) {}

void floor_clean(void *p) {
  Floor *floor = (Floor *)p;
  free(floor);
}
