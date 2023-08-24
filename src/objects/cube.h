#ifndef CUBE_H
#define CUBE_H

#include "../object_lut.h"

#include "../object.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "physics/component.h"
#include "render.h"

typedef struct Cube {
  OBJECT_FIELDS

  PhysComp *phys;
  RenderComp *render;
} Cube;

Cube *cube_build(vec3 position);
void cube_destroy(Cube *c);

void cube_init(void *c);
void cube_update(void *c);
void cube_clean(void *c);

#endif // !CUBE_H
