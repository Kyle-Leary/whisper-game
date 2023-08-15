#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include <GL/gl.h>

void g_set_font(TextureHandle handle) {
  // just pop the passed in handle to the font slot for rendering in the
  // hud_text program.
  glActiveTexture(GL_TEXTURE0 + FONT_TEX_SLOT);
  glBindTexture(GL_TEXTURE_2D, textures[handle]);
}
