#include "shader_instances.h"
#include "global.h"
#include "gui/gui.h"
#include "path.h"
#include "shaders/shader.h"
#include "ubo.h"
#include "whisper/colmap.h"

#include "macros.h"
#include "whisper/hashmap.h"
#include <stdbool.h>

#include "ogl_includes.h"

// use a hashmap so that other renders in the game can easily identify parts of
// themselves with shader configurations here.
static WColMap shader_map = {0};

static bool has_initted = false;

Shader *get_shader(const char *key) {
#ifdef DEBUG
  if (!has_initted) {
    ERROR_NO_ARGS("Tried to grab a shader before init.");
  }
#endif /* ifdef DEBUG */
  Shader *shader_pointer = w_cm_get(&shader_map, key);
  NULL_CHECK_STR_MSG(shader_pointer, key);
  return shader_pointer;
}

#define ID_TO_BLOCK(block_name_literal, block_target_index, shader_id)         \
  {                                                                            \
    unsigned int block_index =                                                 \
        glGetUniformBlockIndex(shader_id, block_name_literal);                 \
    glUniformBlockBinding(shader_id, block_index, block_target_index);         \
  }

#define BIND_LIGHTS()                                                          \
  { ID_TO_BLOCK("LightData", LIGHT_BLOCK, s->id); }
#define BIND_MATRICES()                                                        \
  { ID_TO_BLOCK("ViewProjection", MATRIX_BLOCK, s->id); }
#define BIND_BONES()                                                           \
  { ID_TO_BLOCK("BoneData", BONE_BLOCK, s->id); }

#define GET_SLOT(as)                                                           \
  Shader *s = w_cm_return_slot(&shader_map, as);                               \
  NULL_CHECK_MSG(s, "shader cm return failed, this likely implies a hashmap "  \
                    "collision.\ntry changing the shader name");

#define SHADER_SETUP(shader_name, shader_file)                                 \
  GET_SLOT(shader_name);                                                       \
  s->id = make_shader(SHADER_PATH(shader_file));                               \
  strncpy(s->name, shader_name, 32);                                           \
  glUseProgram(s->id);                                                         \
  shader_general_setup(s);

static void basic_bind(Shader *s) { shader_set_1f(s, "u_time", u_time); }

// the "default" shader for normal 3d scenes
static void init_basic() {
  SHADER_SETUP("basic", "basic.shader");
  shader_set_1i(s, "main_slot", 0);
  // setting a shader function to NULL will just do nothing.
  s->bind = basic_bind;
  s->unbind = NULL;
  BIND_LIGHTS();
  BIND_MATRICES();
}

static void hud_bind(Shader *s) { glDisable(GL_DEPTH_TEST); }
static void hud_unbind(Shader *s) { glEnable(GL_DEPTH_TEST); }

static void init_hud() {
  SHADER_SETUP("hud", "hud.shader");
  shader_set_matrix4fv(s, "projection", (float *)m_ui_projection);
  s->bind = hud_bind;
  s->unbind = hud_unbind;
}

static void hud_text_bind(Shader *s) { glDisable(GL_DEPTH_TEST); }
static void hud_text_unbind(Shader *s) { glEnable(GL_DEPTH_TEST); }

static void init_hud_text() {
  SHADER_SETUP("hud_text", "hud_text.shader");
  shader_set_1i(s, "text_font_slot", FONT_TEX_SLOT);
  shader_set_matrix4fv(s, "projection", (float *)m_ui_projection);
  shader_set_3f(s, "text_base_color", 0.1, 0.5, 0.5);
  s->bind = hud_text_bind;
  s->unbind = hud_text_unbind;
}

static void hud_text_wavy_bind(Shader *s) {
  shader_set_1f(s, "u_time", u_time);
  glDisable(GL_DEPTH_TEST);
}
static void hud_text_wavy_unbind(Shader *s) { glEnable(GL_DEPTH_TEST); }

static void init_hud_text_wavy() {
  SHADER_SETUP("hud_text_wavy", "hud_text_wavy.shader");
  shader_set_1i(s, "text_font_slot", FONT_TEX_SLOT);
  shader_set_matrix4fv(s, "projection", (float *)m_ui_projection);
  shader_set_3f(s, "text_base_color", 0.1, 0.5, 0.5);
  s->bind = hud_text_wavy_bind;
  s->unbind = hud_text_wavy_unbind;
}

static void gouraud_bind(Shader *s) {}

static void init_gouraud() {
  SHADER_SETUP("gouraud", "gouraud.shader");
  s->bind = gouraud_bind;
  s->unbind = NULL;
  BIND_LIGHTS();
  BIND_MATRICES();
}

static void pbr_gouraud_bind(Shader *s) {}

static void init_pbr_gouraud() {
  SHADER_SETUP("pbr_gouraud", "pbr_gouraud.shader");
  s->bind = pbr_gouraud_bind;
  s->unbind = NULL;
  BIND_LIGHTS();
  BIND_MATRICES();
}

static void solid_bind(Shader *s) {
  shader_set_4f(s, "u_render_color", 0, 1, 1, 1);
}

static void init_solid() {
  SHADER_SETUP("solid", "solid.shader");
  s->bind = solid_bind;
  s->unbind = NULL;
  BIND_MATRICES();
}

static void text_3d_bind(Shader *s) {}

static void init_text_3d() {
  SHADER_SETUP("text_3d", "text_3d.shader");
  shader_set_1i(s, "text_font_slot", FONT_TEX_SLOT);
  shader_set_3f(s, "text_base_color", 0.1, 0.5, 0.5);
  s->bind = text_3d_bind;
  s->unbind = NULL;
  BIND_MATRICES();
}

static void skybox_bind(Shader *s) {
  glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
}
static void skybox_unbind(Shader *s) {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
}
// do nothing.
static void skybox_handle_model(Shader *s, mat4 model) {}

static void init_skybox() {
  SHADER_SETUP("skybox", "skybox.shader");
  shader_set_1i(s, "u_cube_tex", SKYBOX_TEX_SLOT);
  s->bind = skybox_bind;
  s->unbind = skybox_unbind;
  s->handle_model = skybox_handle_model;
  BIND_MATRICES();
}

static void model_bind(Shader *s) {}

static void init_model() {
  SHADER_SETUP("model", "model.shader");
  shader_set_1i(s, "tex_sampler", 0);
  s->bind = model_bind;
  s->unbind = NULL;
  BIND_MATRICES();
  BIND_LIGHTS();
  BIND_BONES();
}

// in the immediate mode renderer, point positions are decided entirely by their
// vertex positions and the viewprojection ubo. they ignore the model matrix.
static void im_3d_bind(Shader *s) { glDisable(GL_DEPTH_TEST); }
static void im_3d_unbind(Shader *s) { glEnable(GL_DEPTH_TEST); }
static void im_3d_handle_model(Shader *s, mat4 model) {}

static void init_im_3d() {
  SHADER_SETUP("im_3d", "im_3d.shader");
  s->bind = im_3d_bind;
  s->unbind = im_3d_unbind;
  s->handle_model = im_3d_handle_model;
  BIND_MATRICES();
}

static void gui_bind(Shader *s) {}
static void gui_unbind(Shader *s) {}

static void init_gui() {
  SHADER_SETUP("gui", "gui.shader");
  shader_set_1i(s, "u_tex_sampler", 0); // 0th slot
  s->bind = gui_bind;
  s->unbind = gui_unbind;
}

static void wireframe_bind(Shader *s) {
  glDisable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
static void wireframe_unbind(Shader *s) {
  glEnable(GL_DEPTH_TEST);
  // re-enable the FILL mode, don't do wireframe.
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

static void init_wireframe() {
  SHADER_SETUP("wireframe", "wireframe.shader");
  shader_set_4f(s, "u_render_color", 1, 0, 0, 0.1);
  s->bind = wireframe_bind;
  s->unbind = wireframe_unbind;
  BIND_MATRICES();
}

static void console_bind(Shader *s) {}
static void console_unbind(Shader *s) {}

static void init_console() {
  SHADER_SETUP("console", "console.shader");
  shader_set_1i(s, "u_tex_sampler", 0); // 0th slot
  s->bind = console_bind;
  s->unbind = console_unbind;
}

void shader_instantiate_all() {
  has_initted++;

  w_create_cm(&shader_map, sizeof(Shader), 509);

  {
    // init the pipeline with sensible default settings.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0 +
                    0); // bind slot zero, do this from the offset zero to
                        // get around having to manually put in the enum.
  }

  init_basic();
  init_im_3d();
  init_solid();
  init_model();
  init_skybox();
  init_gouraud();
  init_text_3d();
  init_hud_text();
  init_hud_text_wavy();
  init_hud();
  init_wireframe();
  init_gui();
  init_console();
}

#undef INSERT

#undef APPLY_BINDINGS
#undef BIND_PROGRAMS
#undef ID_TO_BLOCK

void shader_destroy_all() {}
