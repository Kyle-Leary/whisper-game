#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include "global.h"

void g_use_pipeline(PipelineConfiguration config) {
  switch (config) {
  case PC_BASIC:
    glEnable(GL_DEPTH_TEST); // we might be switching from the HUD state, so
                             // reset the depth test.
    glUseProgram(basic_program);

    // set loop uniforms now that the program is bound properly.
    glUniformMatrix4fv(loc_view_tf, 1, GL_FALSE, (float *)m_view_tf);
    glUniformMatrix4fv(loc_view_rot, 1, GL_FALSE, (float *)m_view_rot);
    glUniformMatrix4fv(loc_projection, 1, GL_FALSE, (float *)m_projection);
    break;
  case PC_HUD:                // prepare for hud drawing.
    glDisable(GL_DEPTH_TEST); // dont embed and overlap the ui with other
                              // scene stuff in the 3d shader.

    glUseProgram(hud_program);

    glUniformMatrix4fv(loc_ui_model, 1, GL_FALSE, (float *)m_ui_model);
    glUniformMatrix4fv(loc_ui_projection, 1, GL_FALSE,
                       (float *)m_ui_projection);

    glUniform3f(loc_ui_text_color, 0.5F, 0.5F, 0.5F);
    break;
  default:
    break;
  }
}
