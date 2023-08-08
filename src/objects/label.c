#include "label.h"

#include "../cglm/types.h"
#include "../cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "global.h"
#include "glprim.h"
#include "input_help.h"
#include "meshing/font.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// simple text label for the ui. uses the draw_hud lifecycle functions in the
// LUT.
#define CAST Label *label = (Label *)p

// malloc the text as the callee, don't make the caller do it. let them just
// stack-allocate all their shit.
Label *label_build(vec2 position, const char *text) {
  Label *p = (Label *)malloc(sizeof(Label));
  memcpy(p->position, position, sizeof(float) * 2);
  // maybe make a version of this function that forces static-alloced text, to
  // skip the malloc here entirely?
  unsigned long len = strlen(text);
  p->text = (char *)malloc(len); // world's most obvious buffer overflow
  memcpy(p->text, text,
         len); // holy shit actually using memcpy for its intended purpose
  p->type = OBJ_LABEL;
  // generate the raw buffer, and use this to cache the text.
  p->render = font_mesh_string(ui_font, text, position[0], position[1]);

#ifdef DEBUG
  printf("Made Label object with string [ %s ].\n", p->text);
#endif /* ifdef DEBUG */

  return p;
}

void label_init(void *p) {}

void label_update(void *p) {}

void label_draw_hud(void *p) {
  CAST;
  g_draw_render(label->render);
}

void label_clean(void *p) {
  Label *label = (Label *)p;
  free(label);
}
