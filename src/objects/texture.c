// object that displays a texture on a quad in the HUD.
#include "texture.h"

#include "../cglm/types.h"
#include "../cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/simd/sse2/mat4.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "input_help.h"
#include "meshing/font.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAST Texture *texture = (Texture *)p

Texture *texture_build(AABB aabb, TextureHandle handle) {
  Texture *p = (Texture *)malloc(sizeof(Texture));
  memcpy(&p->aabb, &aabb, sizeof(float) * 4);
  p->ui_render = glprim_ui_rect(p->aabb);
  p->type = OBJ_TEXTURE;
  p->handle = handle;

#ifdef DEBUG
  printf("Made Texture object.\n");
#endif /* ifdef DEBUG */

  return p;
}

void texture_init(void *p) {}

void texture_update(void *p) { CAST; }

void texture_draw_hud(void *p) { // draw under the proper hud context, with the
                                 // right shaders bound and everything.
  CAST;
  g_use_texture(texture->handle);
  glm_mat4_identity(texture->ui_render->model);
  glm_translate(texture->ui_render->model,
                (vec3){texture->aabb.xy[0], texture->aabb.xy[1], 0});
  g_draw_render(texture->ui_render);
}

void texture_clean(void *p) {
  Texture *texture = (Texture *)p;
  free(texture);
}
