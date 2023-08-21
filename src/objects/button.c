#include "button.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "input_help.h"
#include "main.h"
#include "meshing/font.h"
#include "object_lut.h"
#include "objects/label.h"
#include "objects/texture.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// simple text button for the ui. uses the draw_hud lifecycle functions in the
// LUT.
#define CAST Button *button = (Button *)p

// the button has a Texture and Label subobject.
Button *button_build(Font *font, AABB aabb, const char *text,
                     ButtonCallback callback, TextureHandle h) {
  Button *p = (Button *)malloc(sizeof(Button));
  p->type = OBJ_BUTTON;

  p->texture = texture_build(aabb, h);

  // we need to position the text at the center, since that's what the label
  // font meshing function assumes we want.
  vec2 text_center;
  text_center[0] = aabb.xy[0] + (aabb.wh[0] / 2);
  text_center[1] = aabb.xy[1] + (aabb.wh[1] / 2);

  p->label = label_build(font, text_center, text, (vec3){0.1, 0.1, 0.5});

  // just pass NULL if you're not using a callback.
  p->callback = callback;

#ifdef DEBUG
  printf("Made Button object.\n");
#endif /* ifdef DEBUG */

  return p;
}

void button_init(void *p) { CAST; }

void button_update(void *p) {
  CAST;

  if (i_state.act_just_pressed[ACT_HUD_INTERACT]) { // if we're clicking and the
                                                    // point is inside...
    // we get the input mouse pointer state as a double from glfw, downcast to a
    // float to work in the aabb vector calc.
    if (is_point_inside(
            button->texture->aabb,
            (vec2){(float)i_state.pointer[0], (float)i_state.pointer[1]})) {
      // if not NULL. let the caller opt-out of a callback.
      if (button->callback)
        button->callback();
    }
  }
}

void button_clean(void *p) {
  Button *button = (Button *)p;
  fn_lut[OBJ_TEXTURE].clean(button->texture);
  free(button);
}
