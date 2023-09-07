#pragma once

#include "cglm/types.h"
#include "render/render_configuration.h"
#include "shaders/shader_instances.h"

typedef struct GraphicsRender GraphicsRender;

typedef void (*render_setup_fn)(GraphicsRender *gr);

typedef struct GraphicsRender {
  // set this to NULL if you want to opt out of an extra misdirection w/ the
  // callback.
  render_setup_fn setup_fn;

  mat4 model;

  Shader *shader;

  uint vao;   // we only need to explicitly include the VAO, the rest of the
              // buffers are bound to it.
  uint n_idx; // number of indices of the bound vbo to the vao that we'll
              // actually render from the bound ebo.
} GraphicsRender;

void graphics_render_init();

/* pass the g_new_render function one of the *VertexData structure type
 * pointers. it will be resolved and matched based on the configuration. */
GraphicsRender *g_new_render(VertexData *data, const unsigned int *indices,
                             unsigned int i_count);
void g_draw_render(GraphicsRender *graphics_render);
