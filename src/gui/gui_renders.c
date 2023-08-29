#include "gui_renders.h"
#include "cglm/cam.h"
#include "meshing/font.h"
#include "path.h"
#include "render/gl_util.h"

#include "ogl_includes.h"
#include "shaders/shader.h"
#include "shaders/shader_binding.h"
#include <string.h>

GUIRenderState render_state = {0};

static void gui_make_render(GUIRender *gui_render, float *positions, float *uvs,
                            unsigned int *indices, uint num_indices,
                            uint num_verts) { // init test render in opengl.
  gui_render->n_idx = num_indices;
  gui_render->vao = make_vao();
  make_ebo(indices, num_indices);

  make_vbo(positions, num_verts, sizeof(float) * 2);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
  glEnableVertexAttribArray(0);

  make_vbo(uvs, num_verts, sizeof(float) * 2);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void gui_quad_render(GUIRender *gui_render) {
  // Vertex positions (x, y)
  float positions[8] = {
      -0.5f, -0.5f, // Bottom-left corner
      0.5f,  -0.5f, // Bottom-right corner
      0.5f,  0.5f,  // Top-right corner
      -0.5f, 0.5f   // Top-left corner
  };

  // UV coordinates (u, v)
  float uvs[8] = {
      0.0f, 0.0f, // Bottom-left corner
      1.0f, 0.0f, // Bottom-right corner
      1.0f, 1.0f, // Top-right corner
      0.0f, 1.0f  // Top-left corner
  };

  // Indices for the quad
  uint indices[6] = {
      0, 1, 2, // First triangle (bottom-left -> bottom-right -> top-right)
      2, 3, 0  // Second triangle (top-right -> top-left -> bottom-left)
  };

  gui_make_render(gui_render, positions, uvs, indices, 6, 4);
}

void gui_string_render(GUIRender *gui_render,
                       const char *str) { // init test render.
  uint len = strlen(str);

  int num_verts = len * 4;
  int num_indices = len * 6;

  float positions[num_verts * 2];
  float uvs[num_verts * 2];
  uint indices[num_indices];

  font_mesh_string_raw(render_state.font, str, len, 1, 1, positions, uvs,
                       indices);

  gui_make_render(gui_render, positions, uvs, indices, num_indices, num_verts);
}

void gui_render_init() {
  render_state.font =
      font_init(16, 16, g_load_texture(TEXTURE_PATH("ui_font.png")));

  { // init mats/global shader ref from the shader subsystem.
    glm_ortho(0, 1, 0, 1, 0.01, 100, render_state.projection);

    render_state.shader = get_shader("gui");
    shader_bind(render_state.shader);
    shader_set_matrix4fv(render_state.shader, "u_projection",
                         (float *)render_state.projection);
  }
}
