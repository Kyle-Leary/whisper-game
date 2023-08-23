#include "immediate.h"

#include "backends/graphics/graphics_globals.h"
#include "backends/graphics/shader.h"
#include "backends/graphics_api.h"
#include "backends/ogl_includes.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static IMBuffer im_buf = {0};
static Shader *im_3d_shader = NULL;

void im_draw(float *vertices, uint num_vertices, vec4 color, IMDrawMode mode) {
  uint index = im_buf.num_draws; // use the old length as the new index.

  if (im_buf.num_draws > MAX_IM_DRAWS) {
    fprintf(stderr, "ERROR! Could not insert into the immediate drawing list: "
                    "no more room.");
#ifdef DEBUG
    exit(1);
#endif
    return;
  }

  IMDrawCall *c = &(im_buf.draws[im_buf.num_draws]);
  im_buf.num_draws++;

  // setup the vao for drawing later in the flushing function
  GLuint *vao_id = &(im_buf.vaos[index]);
  glGenVertexArrays(1, vao_id);
  glBindVertexArray(*vao_id);

  GLuint vbo_id;
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * num_vertices, vertices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                        (GLvoid *)0);
  glEnableVertexAttribArray(0);

  // fill the found slot in the array with data.
  c->vao_idx = index;
  c->num_vertices = num_vertices;
  c->mode = mode;
  memcpy(c->color, color, sizeof(float) * 4);
}

static GLenum gl_mode_from_drawmode(IMDrawMode m) {
  switch (m) {
  case IM_TRIANGLES: {
    return GL_TRIANGLES;
  } break;
  case IM_POLYGON: {
    return GL_POLYGON;
  } break;
  case IM_LINE_STRIP: {
    return GL_LINE_STRIP;
  } break;
  case IM_LINES: {
    return GL_LINES;
  } break;
  default: {
    return GL_LINE_STRIP;
  } break;
  }
}

static void im_flush_one(IMDrawCall *call) {
  shader_set_4f(im_3d_shader, "u_color", call->color[0], call->color[1],
                call->color[2], call->color[3]);
  glBindVertexArray(im_buf.vaos[call->vao_idx]);
  glDrawArrays(gl_mode_from_drawmode(call->mode), 0, call->num_vertices);
}

void im_init() {
  im_3d_shader = w_hm_get(shader_map, "im_3d").as_ptr;
  assert(im_3d_shader != NULL);
  im_buf.num_draws = 0;
}

void im_flush() {
  for (int i = 0; i < im_buf.num_draws; i++) {
    IMDrawCall *draw = &(im_buf.draws[i]);
    im_flush_one(draw);
  }

  // all in one call!!!
  glDeleteVertexArrays(im_buf.num_draws, im_buf.vaos);

  im_buf.num_draws = 0;
}
