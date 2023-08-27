#include "gl_util.h"

#include "ogl_includes.h"

// would it be worth it to pre-generate buffers and then pool them out here?
// that way we could batch the gl bufgen calls into chunks.
uint make_vbo(const void *data, unsigned int num_verts,
              unsigned int sizeof_vtx) {
  uint render_id;
  glGenBuffers(1, &render_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof_vtx * num_verts, data,
               GL_STATIC_DRAW); // we could actually get away with passing this
                                // stack-allocated vertices, since we're calling
                                // this IN the new_Render function anyway?
  return render_id;
}

uint make_ebo(const unsigned int *indices, unsigned int count) {
  uint render_id;
  glGenBuffers(1, &render_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices,
               GL_STATIC_DRAW);
  return render_id;
}

uint make_vao() {
  uint render_id;
  glGenVertexArrays(1, &render_id); // make the vao
  glBindVertexArray(
      render_id); // note; each of the make_ functions in this source file
                  // implicitly bind the object in the context.
  return render_id;
}
