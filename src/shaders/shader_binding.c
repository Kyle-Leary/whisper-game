#include "shader_binding.h"
#include "shaders/shader.h"

// since they're statically inserted into the hashmap at instantiation time, we
// can use the pointer to the shader as a safe comparator. could also maybe use
// the hash?
static Shader *curr_sh;

void shader_bind(Shader *s) {
  shader_use(s);
  if (s->bind)
    s->bind();
}

void shader_unbind(Shader *s) {
  if (s->unbind)
    s->unbind();
}
