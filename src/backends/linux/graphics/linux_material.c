#include "backends/graphics_api.h"
#include "backends/linux/graphics/shader.h"
#include "backends/linux/ogl_includes.h"
#include "linux_graphics_globals.h"
#include <GL/gl.h>
#include <stdio.h>

// use the material, in this case that means setting all the right values on the
// active shader.
void g_use_material(MaterialData *mat) {
  if (mat == NULL) {
    fprintf(stderr,
            "g_use_material() was passed a NULL material. Returning...\n");
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, material_data_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialData), mat);
}

void g_use_pbr_texture(TextureType type, TextureHandle tex) {
  glActiveTexture(GL_TEXTURE0 + type);
  glBindTexture(GL_TEXTURE_2D, tex);
}
