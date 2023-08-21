#ifndef FLOOR_H
#define FLOOR_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "render.h"

typedef struct Floor {
  OBJECT_FIELDS

  PhysComp *phys;
  RenderComp *render;

  float speed;
} Floor;

Floor *floor_build(vec3 position, float strength);
void floor_destroy(Floor *f);

void floor_init(void *f);
void floor_update(void *f);
void floor_clean(void *f);
void floor_handle_collision(void *f, CollisionEvent *e);

#endif // !FLOOR_H
