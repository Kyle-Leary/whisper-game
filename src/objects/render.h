#ifndef render_H
#define render_H

// just a wrapper type around a single graphicsrender.

#include "../object_lut.h"

#include "../cglm/types.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"

typedef struct RenderObject {
  ObjectType type;
  Collider *colliders;

  // caller should generically modify the model matrix of the render to change
  // the position. this is just a wrapper!!
  GraphicsRender *render;
} RenderObject;

RenderObject *render_build(GraphicsRender *render);
void render_destroy(RenderObject *c);

void render_draw(void *c);
void render_clean(void *c);

#endif // !render_H
