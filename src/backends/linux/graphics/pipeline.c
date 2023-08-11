#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include "backends/linux/graphics/shader.h"
#include "global.h"
#include "linux_graphics_globals.h"
#include "util.h"
#include "whisper/hashmap.h"
#include <GL/gl.h>

#define GETSH(name) (w_hm_get(shader_map, name).as_ptr)

static PipelineConfiguration curr_pc;

// called when a stage is left, when the internal stage is properly changed to
// another one through the g_use_pipeline function.
static void leave_stage(PipelineConfiguration config) {
  switch (config) {
  case PC_HUD: {
    glEnable(GL_DEPTH_TEST);
  } break;
  case PC_SKYBOX: {
    // do more weird depth testing stuff in the skybox. this time, properly
    // change the depth functions rather than just turning the depth buffer off
    // in rendering.
    glDepthFunc(GL_LESS);
  } break;
  default: {
  } break;
  }
}

void g_use_pipeline(PipelineConfiguration config) {
  if (curr_pc != config) {
    leave_stage(curr_pc);
    curr_pc = config;
  }

  switch (config) {
  case PC_BASIC: {
    glEnable(GL_DEPTH_TEST); // we might be switching from the HUD state, so
                             // reset the depth test.

    shader_set_matrix4fv(w_hm_get(shader_map, "basic").as_ptr, "view",
                         (const float *)m_view);
    shader_set_matrix4fv(w_hm_get(shader_map, "basic").as_ptr, "projection",
                         (const float *)m_projection);
  } break;
  case PC_HUD: {              // prepare for hud drawing.
    glDisable(GL_DEPTH_TEST); // dont embed and overlap the ui with other
                              // scene stuff in the 3d shader.

    shader_set_matrix4fv(w_hm_get(shader_map, "hud").as_ptr, "model",
                         (const float *)m_ui_model);
    shader_set_matrix4fv(w_hm_get(shader_map, "hud").as_ptr, "projection",
                         (const float *)m_ui_projection);

    // shader_set_3f(w_hm_get(shader_map, "hud").as_ptr, "ui_text_color", 0.5F,
    //               0.5F, 0.5F);
  } break;
  case PC_BLANK_GOURAUD: {
    glEnable(GL_DEPTH_TEST);

    shader_set_matrix4fv(w_hm_get(shader_map, "gouraud").as_ptr, "view",
                         (const float *)m_view);
    shader_set_matrix4fv(w_hm_get(shader_map, "gouraud").as_ptr, "projection",
                         (const float *)m_projection);
  } break;
  case PC_PBR_GOURAUD: {
    glEnable(GL_DEPTH_TEST);

    shader_set_matrix4fv(w_hm_get(shader_map, "pbr_gouraud").as_ptr, "view",
                         (const float *)m_view);
    shader_set_matrix4fv(w_hm_get(shader_map, "pbr_gouraud").as_ptr,
                         "projection", (const float *)m_projection);
  } break;
  case PC_SOLID: {
    Shader *ptr = GETSH("solid");

    shader_set_matrix4fv(ptr, "view", (const float *)m_view);
    shader_set_matrix4fv(ptr, "projection", (const float *)m_projection);
    // we can set a "default" color here.
    shader_set_3f(ptr, "u_render_color", 0, 1, 0);
  } break;
  case PC_SKYBOX: {
    Shader *ptr = GETSH("skybox");

    shader_set_matrix4fv(ptr, "view", (const float *)m_view);
    shader_set_matrix4fv(ptr, "projection", (const float *)m_projection);

    shader_set_1i(ptr, "u_cube_tex", 0);

    glDepthFunc(GL_LEQUAL);
  } break;
  default: {
  } break;
  }
}
