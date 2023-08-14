#ifndef BUTTON_H
#define BUTTON_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "helper_math.h"
#include "objects/texture.h"

// should this take in anything?
typedef void (*ButtonCallback)();

typedef struct Button {
  ObjectType type;
  Collider *colliders;

  ButtonCallback callback;
  Texture *texture;
} Button;

Button *button_build(AABB aabb, const char *text, ButtonCallback callback,
                     TextureHandle h);
void button_destroy(Button *b);

void button_init(void *b);
void button_update(void *b);
void button_draw(void *b);
void button_clean(void *b);

#endif // !BUTTON_H
