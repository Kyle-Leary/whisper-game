#include "vao.h"
#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include "backends/linux/graphics/render_structs.h"
#include "global.h"

// we will assume that the caller has already bound their generated vao, and
// just wants to set the proper attrib links between the bound vao and bound ebo
// and vbo.
void vao_basic_config(void *data) {
  // make the assumption that the caller will bind with the proper structure.
  BasicVertexData *basic = (BasicVertexData *)data;

  // recall that the make_vbo function also implicitly binds the vbo.
  make_vbo(basic->position, basic->v_count, sizeof(float) * 3);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)0);
  glEnableVertexAttribArray(0);

  make_vbo(basic->normal, basic->v_count, sizeof(float) * 3);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)0);
  glEnableVertexAttribArray(1);

  make_vbo(basic->uv, basic->v_count, sizeof(float) * 2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
  glEnableVertexAttribArray(2);

  // TODO: take in the vbo indices, free them here?
}

void vao_model_config(void *data) {
  vao_basic_config(data);
  ModelVertexData *model = (ModelVertexData *)data;

  // ivec4 in glsl
  make_vbo(model->joint, model->v_count, sizeof(int) * 4);
  glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, sizeof(int) * 4, (void *)0);
  glEnableVertexAttribArray(3);

  make_vbo(model->weight, model->v_count, sizeof(float) * 4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *)0);
  glEnableVertexAttribArray(4);
}

void vao_hud_config(void *data) {
  HUDVertexData *hud = (HUDVertexData *)data;

  make_vbo(hud->position, hud->v_count, sizeof(float) * 2);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
  glEnableVertexAttribArray(0);

  make_vbo(hud->uv, hud->v_count, sizeof(float) * 2);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
  glEnableVertexAttribArray(1);
}

// TODO: should this be linked with the shader? shaders aren't necessarily
// one-to-one with render configurations.
//
// just like the object table, define the
// render configurations in a quick way based on enum variants.
RenderConfigurationData render_config_table[RC_COUNT] = {
    [RC_BASIC] = {.conf_func = vao_basic_config,
                  .sizeof_vtx = SIZEOF_BASIC_VTX},
    [RC_HUD] = {.conf_func = vao_hud_config, .sizeof_vtx = SIZEOF_HUD_VTX},
    [RC_MODEL] = {.conf_func = vao_model_config,
                  .sizeof_vtx = SIZEOF_MODEL_VTX},
};
