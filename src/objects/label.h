#ifndef LABEL_H
#define LABEL_H

#include "../object_lut.h"

#include "../cglm/types.h"
#include "../object.h"
#include "../physics.h"

#include "backends/graphics_api.h"

typedef struct Label {
  ObjectType type;
  Collider *colliders;

  vec2 position;
  char *text;
  Render *render;
} Label;

Label *label_build(vec2 position, const char *text);
void label_destroy(Label *l);

void label_init(void *l);
void label_update(void *l);
void label_draw_hud(void *l);
void label_clean(void *l);

#endif // !LABEL_H
