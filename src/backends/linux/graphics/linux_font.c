#include "../ogl_includes.h"
#include "backends/graphics_api.h"

void g_set_font(TextureHandle handle) {
  // just pop the passed in handle to the font slot for rendering in the
  // hud_text program.
  glActiveTexture(FONT_TEX_SLOT);
  glBindTexture(GL_TEXTURE_2D, textures[handle]);
}
