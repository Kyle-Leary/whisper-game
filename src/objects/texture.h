#ifndef TEXTURE_H
#define TEXTURE_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "helper_math.h"

typedef struct Texture {
  ObjectType type;
  Collider *colliders;

  AABB aabb;
  GraphicsRender *render; // Render is generic enough that it'll work for either
                          // rendering type. it's just an ebo and vbo, nothing
                          // special. it doesn't inherently make any assumptions
                          // about the shape of the vertices going into the
                          // Render, or the underlying shader context.
  TextureHandle handle;
} Texture;

Texture *texture_build(AABB aabb, TextureHandle handle);
void texture_destroy(Texture *t);

void texture_init(void *t);
void texture_draw(void *t);
void texture_update(void *t);
void texture_clean(void *t);

#endif // !TEXTURE_H
