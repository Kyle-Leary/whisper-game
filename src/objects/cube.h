#ifndef CUBE_H
#define CUBE_H

#include "../object_lut.h"

#include "cglm/types.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"

typedef struct Cube {
  PHYS_OBJECT_FIELDS

  GraphicsRender *render;
  float speed;
} Cube;

Cube *cube_build(vec3 position);
void cube_destroy(Cube *c);

void cube_init(void *c);
void cube_update(void *c);
void cube_draw(void *c);
void cube_clean(void *c);
void cube_handle_collision(void *c, CollisionEvent *e);

#endif // !CUBE_H
