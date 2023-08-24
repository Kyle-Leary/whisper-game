#ifndef TEXTURE_H
#define TEXTURE_H

#include "../object_lut.h"

#include "../object.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "helper_math.h"
#include "object_bases.h"

typedef struct Texture {
  OBJECT_FIELDS

  AABB aabb;
  GraphicsRender *render;

  TextureHandle handle;
} Texture;

Texture *texture_build(AABB aabb, TextureHandle handle);
void texture_destroy(Texture *t);

void texture_init(void *t);
void texture_update(void *t);
void texture_clean(void *t);

#endif // !TEXTURE_H
