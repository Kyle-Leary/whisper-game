#ifndef render_H
#define render_H

// just a wrapper type around a single graphicsrender.

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"

typedef void (*RenderSetup)();

typedef struct RenderObject {
  ObjectType type;
  Collider *colliders;

  // caller should generically modify the model matrix of the render to change
  // the position. this is just a wrapper!!
  GraphicsRender *render;
  RenderSetup setup;
} RenderObject;

RenderObject *render_build(GraphicsRender *render, RenderSetup setup);
void render_destroy(RenderObject *c);

void render_draw(void *c);
void render_clean(void *c);

#endif // !render_H
