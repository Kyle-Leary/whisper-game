#pragma once

#include "meshing/font.h"
#include "shaders/shader_instances.h"
#include <sys/types.h>

typedef struct GUIRenderState {
  Font *font;
  Shader *shader;
  mat4 projection;
} GUIRenderState;

typedef struct GUIRender {
  uint vao;
  uint n_idx;
} GUIRender;

extern GUIRenderState render_state;

void gui_quad_render(GUIRender *gui_render);
void gui_string_render(GUIRender *gui_render, const char *str);

void gui_render_init();
