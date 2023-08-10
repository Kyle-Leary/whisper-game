#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include "backends/linux/graphics/shader.h"
#include "global.h"
#include "linux_graphics_globals.h"

void g_use_pipeline(PipelineConfiguration config) {
  switch (config) {
  case PC_BASIC: {
    glEnable(GL_DEPTH_TEST); // we might be switching from the HUD state, so
                             // reset the depth test.

    shader_set_matrix4fv(basic_program, "view", (const float *)m_view);
    shader_set_matrix4fv(basic_program, "projection",
                         (const float *)m_projection);
  } break;
  case PC_HUD: {              // prepare for hud drawing.
    glDisable(GL_DEPTH_TEST); // dont embed and overlap the ui with other
                              // scene stuff in the 3d shader.

    shader_set_matrix4fv(hud_program, "model", (const float *)m_ui_model);
    shader_set_matrix4fv(hud_program, "projection",
                         (const float *)m_ui_projection);

    shader_set_3f(hud_program, "ui_text_color", 0.5F, 0.5F, 0.5F);
  } break;
  case PC_BLANK_GOURAUD: {
    glEnable(GL_DEPTH_TEST);

    shader_set_matrix4fv(gouraud_program, "view", (const float *)m_view);
    shader_set_matrix4fv(gouraud_program, "projection",
                         (const float *)m_projection);
  } break;
  default:
    break;
  }
}
