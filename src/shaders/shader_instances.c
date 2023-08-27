#include "shader_instances.h"
#include "backends/ogl_includes.h"
#include "global.h"
#include "path.h"
#include "shaders/shader.h"
#include "ubo.h"
#include "whisper/colmap.h"

#include "macros.h"

// use a hashmap so that other renders in the game can easily identify parts of
// themselves with shader configurations here.
WColMap shader_map = {0};

#define ID_TO_BLOCK(block_name_literal, block_target_index, shader_id)         \
  {                                                                            \
    unsigned int block_index =                                                 \
        glGetUniformBlockIndex(shader_id, block_name_literal);                 \
    printf("found block index %d\n", block_index);                             \
    glUniformBlockBinding(shader_id, block_index, block_target_index);         \
  }

#define BIND_LIGHTS()                                                          \
  { ID_TO_BLOCK("LightData", LIGHT_BLOCK, s->id); }
#define BIND_MATRICES()                                                        \
  { ID_TO_BLOCK("ViewProjection", MATRIX_BLOCK, s->id); }
#define BIND_BONES()                                                           \
  { ID_TO_BLOCK("BoneData", BONE_BLOCK, s->id); }

#define GET_SLOT(as) Shader *s = w_cm_return_slot(&shader_map, as);

#define SHADER_SETUP(shader_name, shader_file)                                 \
  GET_SLOT(shader_name);                                                       \
  s->id = make_shader(SHADER_PATH(shader_file));                               \
  shader_general_setup(s);

static void basic_bind(Shader *s) { shader_set_1f(s, "u_time", u_time); }

// the "default" shader for normal 3d scenes
static void init_basic() {
  SHADER_SETUP("basic", SHADER_PATH("basic.shader"));
  shader_set_1i(s, "main_slot", 0);
  // setting a shader function to NULL will just do nothing.
  s->bind = basic_bind;
  s->unbind = NULL;
  BIND_LIGHTS();
  BIND_MATRICES();
}

static void hud_text_bind(Shader *s) {}

static void init_hud_text() {
  SHADER_SETUP("hud_text", SHADER_PATH("hud_text.shader"));
  shader_set_1i(s, "text_font_slot", FONT_TEX_SLOT);
  shader_set_matrix4fv(s, "projection", (float *)m_ui_projection);
  shader_set_3f(s, "text_base_color", 0.1, 0.5, 0.5);
  s->bind = hud_text_bind;
  s->unbind = NULL;
}

static void hud_text_wavy_bind(Shader *s) {}

static void init_hud_text_wavy() {
  SHADER_SETUP("hud_text_wavy", SHADER_PATH("hud_text_wavy.shader"));
  shader_set_1i(s, "text_font_slot", FONT_TEX_SLOT);
  shader_set_matrix4fv(s, "projection", (float *)m_ui_projection);
  shader_set_3f(s, "text_base_color", 0.1, 0.5, 0.5);
  s->bind = hud_text_wavy_bind;
  s->unbind = NULL;
}

static void gouraud_bind(Shader *s) {}

static void init_gouraud() {
  SHADER_SETUP("gouraud", SHADER_PATH("gouraud.shader"));
  s->bind = gouraud_bind;
  s->unbind = NULL;
  BIND_LIGHTS();
  BIND_MATRICES();
}

static void pbr_gouraud_bind(Shader *s) {}

static void init_pbr_gouraud() {
  SHADER_SETUP("pbr_gouraud", SHADER_PATH("pbr_gouraud.shader"));
  s->bind = pbr_gouraud_bind;
  s->unbind = NULL;
  BIND_LIGHTS();
  BIND_MATRICES();
}

static void solid_bind(Shader *s) {}

static void init_solid() {
  SHADER_SETUP("solid", SHADER_PATH("solid.shader"));
  s->bind = solid_bind;
  s->unbind = NULL;
  BIND_MATRICES();
}

static void text_3d_bind(Shader *s) {}

static void init_text_3d() {
  SHADER_SETUP("text_3d", SHADER_PATH("text_3d.shader"));
  shader_set_1i(s, "text_font_slot", FONT_TEX_SLOT);
  shader_set_3f(s, "text_base_color", 0.1, 0.5, 0.5);
  s->bind = text_3d_bind;
  s->unbind = NULL;
  BIND_MATRICES();
}

static void skybox_bind(Shader *s) {}

static void init_skybox() {
  SHADER_SETUP("skybox", SHADER_PATH("skybox.shader"));
  shader_set_1i(s, "text_font_slot", FONT_TEX_SLOT);
  shader_set_3f(s, "text_base_color", 0.1, 0.5, 0.5);
  s->bind = skybox_bind;
  s->unbind = NULL;
  BIND_MATRICES();
}

static void model_bind(Shader *s) {}

static void init_model() {
  SHADER_SETUP("model", SHADER_PATH("model.shader"));
  shader_set_1i(s, "tex_sampler", 0);
  s->bind = model_bind;
  s->unbind = NULL;
  BIND_MATRICES();
  BIND_LIGHTS();
  BIND_BONES();
}

static void im_3d_bind(Shader *s) {}

static void init_im_3d() {
  SHADER_SETUP("im_3d", SHADER_PATH("im_3d.shader"));
  s->bind = im_3d_bind;
  s->unbind = NULL;
  BIND_MATRICES();
}

void shader_instantiate_all() {
  init_basic();
  init_im_3d();
  init_solid();
  init_model();
  init_skybox();
  init_gouraud();
  init_pbr_gouraud();
  init_text_3d();
  init_hud_text();
  init_hud_text_wavy();
}

#undef INSERT

#undef APPLY_BINDINGS
#undef BIND_PROGRAMS
#undef ID_TO_BLOCK

void shader_destroy_all() {}
