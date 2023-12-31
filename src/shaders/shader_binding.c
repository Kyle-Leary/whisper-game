#include "shader_binding.h"
#include "macros.h"
#include "shaders/shader.h"

#include "ogl_includes.h"

// since they're statically inserted into the hashmap at instantiation time, we
// can use the pointer to the shader as a safe comparator. could also maybe use
// the hash?
static Shader *curr_sh = NULL;

static int last_was_wireframe = 0;

void shader_bind(Shader *s) {
  NULL_CHECK(s);

  if (last_was_wireframe) {
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  if (s->is_wireframe) {
    last_was_wireframe = 1;
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  if (s != curr_sh) {
    // new shader, so unbind the old one.
    if (curr_sh && curr_sh->unbind) {
      curr_sh->unbind(s);
    }

    // new shader.
    // use the shader before calling the bind function, since it might set
    // uniforms that depend on the shader being active to set properly.
    glUseProgram(s->id);
    if (s->bind)
      s->bind(s);
    curr_sh = s;
  }
}

void shader_handle_model(Shader *s, mat4 model) {
  if (s->handle_model) {
    s->handle_model(s, model);
  } else {
    // default behavior if NULL.
    shader_set_matrix4fv(s, "u_model", (float *)model);
  }
}
