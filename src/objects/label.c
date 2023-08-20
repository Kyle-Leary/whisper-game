#include "label.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/affine-pre.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
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
Label *label_build(Font *font, vec2 position, const char *text, vec3 color) {
  Label *p = (Label *)malloc(sizeof(Label));
  memcpy(p->position, position, sizeof(float) * 2);

  { // setup the text, malloc space for the string in the Label and copy the
    // text data into the pointer.
    unsigned long len = strlen(text);
    p->text = (char *)malloc(len + 1);
    memcpy(p->text, text, len);
    p->text[len] = '\0';
  }

  p->type = OBJ_LABEL;
  // generate the raw buffer, and use this to cache the text.
  p->render = font_mesh_string(font, text, 0.03, 0.05);

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

void label_init(void *p) {}

void label_update(void *p) {}

void label_draw(void *p) {
  CAST;
  // setup font rendering with the attached font.
  g_use_texture(label->font->tex_handle, FONT_TEX_SLOT);

  { // the outline pass
    g_set_font_color((vec3){1, 1, 1});
    glm_translate(label->render->model,
                  (vec3){label->position[0], label->position[1], 0});
    g_draw_render(label->render);
    glm_mat4_identity(label->render->model);
  }

  { // the center text rendering pass
    g_set_font_color(label->color);
    glm_translate(label->render->model,
                  (vec3){label->position[0], label->position[1], 0});
    glm_scale(label->render->model, (vec3){0.99, 0.9, 1});
    glm_translate(label->render->model, (vec3){0.003, -0.002, 0});
    g_draw_render(label->render);
    glm_mat4_identity(label->render->model);
  }
}

void label_clean(void *p) {
  Label *label = (Label *)p;
  free(label);
}
