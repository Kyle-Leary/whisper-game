#ifndef LABEL_H
#define LABEL_H

#include "../object_lut.h"

#include "../object.h"
#include "cglm/types.h"

#include "backends/graphics_api.h"
#include "meshing/font.h"
#include "object_bases.h"
#include "render.h"

#define LABEL_TEXT_SZ 512

typedef struct Label {
  OBJECT_FIELDS

  RenderComp *render;

  // don't force the caller to figure out the max width and height required for
  // the text. just specify an origin point for the text to be fixed from.
  vec2 position;
  Font *font;
  vec3 color; // tint the text color before rendering.
  char text[LABEL_TEXT_SZ];
} Label;

Label *label_build(Font *font, vec2 position, const char *text, vec3 color);
void label_destroy(Label *l);

void label_init(void *l);
void label_change_text(Label *label, const char *text);
void label_update(void *l);
void label_clean(void *l);

#endif // !LABEL_H
