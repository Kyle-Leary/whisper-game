// object that displays a texture on a quad in the HUD.
#include "texture.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/simd/sse2/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "input_help.h"
#include "meshing/font.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAST Texture *texture = (Texture *)p

Texture *texture_build(AABB aabb, TextureHandle handle) {
  Texture *p = (Texture *)malloc(sizeof(Texture));
  memcpy(&p->aabb, &aabb, sizeof(float) * 4);
  p->render = glprim_ui_rect(p->aabb, false);
  p->type = OBJ_TEXTURE;
  p->handle = handle;

  p->render->pc = PC_HUD;

#ifdef DEBUG
  printf("Made Texture object.\n");
#endif /* ifdef DEBUG */

  return p;
}

void texture_init(void *p) {}

void texture_update(void *p) { CAST; }

void texture_draw(void *p) { // draw under the proper hud context, with the
                             // right shaders bound and everything.
  CAST;
  glm_mat4_identity(texture->render->model);
  glm_translate(texture->render->model,
                (vec3){texture->aabb.xy[0], texture->aabb.xy[1], 0});
  g_use_texture(texture->handle, 0);
  g_draw_render(texture->render);
}

void texture_clean(void *p) {
  Texture *texture = (Texture *)p;
  free(texture);
}
