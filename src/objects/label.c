#include "label.h"

#include "../object.h"
#include "backends/graphics_api.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"
#include "input_help.h"
#include "meshing/font.h"
#include "render.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// simple text label for the ui. uses the draw_hud lifecycle functions in the
// LUT.
#define CAST Label *label = (Label *)p

// malloc the text as the callee, don't make the caller do it. let them just
// stack-allocate all their shit.
Label *label_build(Font *font, vec2 position, const char *text, vec3 color) {
  Label *p = (Label *)malloc(sizeof(Label));
  p->type = OBJ_LABEL;

  memcpy(p->position, position, sizeof(float) * 2);

  { // setup the text, malloc space for the string in the Label and copy the
    // text data into the pointer.
    int len = strlen(text);
    memcpy(p->text, text, len);
    p->text[len] = '\0';
  }

  // generate the raw buffer, and use this to cache the text.
  p->render = make_rendercomp_from_graphicsrender(
      font_mesh_string(font, text, 0.03, 0.05));

  memcpy(p->color, color, sizeof(float) * 3);
  p->font = font; // the font also contains the font texture we'll use for
                  // rendering later. additionally, if we want to change the
                  // GraphicsRender if the text changes, we can do that with the
                  // attached font.

#ifdef DEBUG
  printf("Made Label object with string [ %s ].\n", p->text);
#endif /* ifdef DEBUG */

  return p;
}

void label_change_text(Label *label, const char *text) {
  if (strcmp(label->text, text) == 0) {
    // we're already rendering this text, then.
    return;
  }

  { // re-setup the text inside the label to be the new string
    int len = strlen(text);
    memcpy(label->text, text, len);
    label->text[len] = '\0';
  }

  { // re-generate the text render. swap out the render data with the new
    // GraphicsRender and free the old one.

    label->render->data = font_mesh_string(label->font, text, 0.03, 0.05);
  }
}

void label_init(void *p) {}

void label_update(void *p) {
  CAST;

  GraphicsRender *prim = label->render->data;

  g_use_texture(label->font->tex_handle, FONT_TEX_SLOT);

  glm_mat4_identity(prim->model);
  glm_translate(prim->model, (vec3){label->position[0], label->position[1], 0});
}

void label_clean(void *p) {
  Label *label = (Label *)p;
  free(label);
}
