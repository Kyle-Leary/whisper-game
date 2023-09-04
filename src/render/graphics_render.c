#include "graphics_render.h"
#include "cglm/mat4.h"
#include "im_prims.h"
#include "macros.h"
#include "render/gl_util.h"
#include "render/render_configuration.h"
#include "shaders/shader_binding.h"

#include "ogl_includes.h"
#include "whisper/array.h"

static WArray gr_pool = {0};

void graphics_render_init() {
  w_make_array(&gr_pool, sizeof(GraphicsRender), 1000);
}

GraphicsRender *g_new_render(VertexData *data, const unsigned int *indices,
                             unsigned int i_count) {
  GraphicsRender r;
  r.n_idx = i_count;

  { // setup the vao on the graphicsrender, bind all the stuff to the vao and
    // only keep the vao around.
    r.vao = make_vao();
    make_ebo(indices, i_count);
    configure_render(data);
  }

  glm_mat4_identity(r.model);

  GraphicsRender *final_slot = w_array_insert(&gr_pool, &r);
  NULL_CHECK(final_slot);

  return final_slot;
}

// mutate the model per-object.
void g_draw_render(GraphicsRender *graphics_render) {
  NULL_CHECK(graphics_render);

  shader_bind(graphics_render->shader);
  shader_handle_model(graphics_render->shader, graphics_render->model);

  im_transform(graphics_render->model);

  glBindVertexArray(graphics_render->vao);
  glDrawElements(GL_TRIANGLES, graphics_render->n_idx, GL_UNSIGNED_INT, 0);
}
