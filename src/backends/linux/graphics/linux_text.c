#include "backends/linux/graphics/shader.h"
#include "cglm/types.h"
#include "linux_graphics_globals.h"

void g_set_font_color(vec3 color) {
  Shader *text_shader = w_hm_get(shader_map, "hud_text").as_ptr;
  shader_set_3f(text_shader, "text_base_color", color[0], color[1], color[2]);
}
