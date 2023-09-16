#include "material.h"
#include "render/texture.h"

void g_use_material(Material *mat) {
  g_use_texture(mat->base_color_texture, 0);
}

void free_material(Material *mat) {
  // ??? what to do? maybe free the inner textures?
}
