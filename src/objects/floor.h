#ifndef FLOOR_H
#define FLOOR_H

#include "../object_lut.h"

#include "../object.h"
#include "cglm/types.h"
#include "physics/component.h"
#include "render/render.h"

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

#endif // !FLOOR_H
