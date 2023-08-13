#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include "linux_graphics_globals.h"

// use SubData to copy it in.
void g_use_bones(mat4 *bones, int num_bones) {
  glBindBuffer(GL_UNIFORM_BUFFER, bone_data_ubo);
  int bone_sz = sizeof(float) * 16 * BONE_LIMIT;
  glBufferSubData(GL_UNIFORM_BUFFER, 0, bone_sz, bones);
  glBufferSubData(GL_UNIFORM_BUFFER, bone_sz, sizeof(int), &num_bones);
}
