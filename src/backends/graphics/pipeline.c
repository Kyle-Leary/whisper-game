#include "../ogl_includes.h"
#include "backends/graphics/shader.h"
#include "backends/graphics_api.h"
#include "global.h"
#include "graphics_globals.h"
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
  case PC_HUD_TEXT: {
    glEnable(GL_DEPTH_TEST);
  } break;
  case PC_HUD_TEXT_WAVY: {
    glEnable(GL_DEPTH_TEST);
  } break;
  case PC_WIREFRAME: {
    // re-enable the FILL mode, don't do wireframe.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } break;
  case PC_SKYBOX: {
    // do more weird depth testing stuff in the skybox. this time, properly
    // change the depth functions rather than just turning the depth buffer off
    // in rendering.
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
  } break;
  default: {
  } break;
  }
}

void g_use_pipeline(PipelineConfiguration config) {
  if (curr_pc != config) {
    leave_stage(curr_pc);
    curr_pc = config;
  } else {
    // attempted to do a redundant shader configuration, just ignore it.
    return;
  }

  switch (config) {
  case PC_INVALID: {
    fprintf(stderr, "ERROR: invalid pipeline configuration.\n");
    exit(1);
  } break;
  case PC_BASIC: {
    Shader *ptr = GETSH("basic");
    shader_set_1f(ptr, "u_time", u_time);
  } break;
  case PC_HUD: {              // prepare for hud drawing.
    glDisable(GL_DEPTH_TEST); // dont embed and overlap the ui with other
                              // scene stuff in the 3d shader.

    Shader *sh = w_hm_get(shader_map, "hud").as_ptr;
    shader_use(sh);
  } break;
  case PC_HUD_TEXT: { // prepare for hud drawing.
    glDisable(GL_DEPTH_TEST);
    Shader *sh = w_hm_get(shader_map, "hud_text").as_ptr;
    shader_use(sh);
  } break;
  case PC_HUD_TEXT_WAVY: {
    glDisable(GL_DEPTH_TEST);
    Shader *sh = w_hm_get(shader_map, "hud_text_wavy").as_ptr;
    shader_set_1f(sh, "u_time", u_time);
  } break;
  case PC_WIREFRAME: {
    // wireframe is mostly just handled through the settings, not the shader.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Shader *ptr = GETSH("solid");
    shader_use(ptr);

    shader_set_3f(ptr, "u_render_color", 1, 0, 0);
  } break;
  case PC_BLANK_GOURAUD: {
    shader_use(GETSH("gouraud"));

  } break;
  case PC_PBR_GOURAUD: {
    shader_use(GETSH("pbr_gouraud"));

  } break;
  case PC_SOLID: {
    Shader *ptr = GETSH("solid");

    // we can set a "default" color here.
    shader_set_3f(ptr, "u_render_color", 0, 1, 0);
  } break;
  case PC_TEXT_3D: {
    Shader *ptr = GETSH("text_3d");
    shader_use(ptr);
  } break;
  case PC_SKYBOX: {
    Shader *ptr = GETSH("skybox");
    shader_set_1i(ptr, "u_cube_tex", 0);

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  } break;
  case PC_MODEL: {
    Shader *ptr = GETSH("model");

    shader_use(ptr);
  } break;
  case PC_IMMEDIATE_3D: {
    Shader *ptr = GETSH("im_3d");
    shader_use(ptr);
  } break;
  default: {
  } break;
  }
}
