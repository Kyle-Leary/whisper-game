#ifndef LABEL_H
#define LABEL_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "cglm/types.h"

#include "backends/graphics_api.h"
#include "meshing/font.h"
#include "object_bases.h"

typedef struct Label {
  OBJECT_FIELDS

  // don't force the caller to figure out the max width and height required for
  // the text. just specify an origin point for the text to be fixed from.
  vec2 position;
  char *text;
  GraphicsRender *render;
  Font *font;
  vec3 color; // tint the text color before rendering.
} Label;

Label *label_build(Font *font, vec2 position, const char *text, vec3 color);
void label_destroy(Label *l);

void label_init(void *l);
void label_update(void *l);
void label_draw(void *l);
void label_clean(void *l);

#endif // !LABEL_H
