#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include "linux_graphics_globals.h"

// use SubData to copy it in.
void g_use_bones(mat4 *bones, mat4 *ibms, int num_bones) {
  glBindBuffer(GL_UNIFORM_BUFFER, bone_data_ubo);
  int bone_sz = sizeof(float) * 16 * BONE_LIMIT;
  int ibm_sz = bone_sz;
  glBufferSubData(GL_UNIFORM_BUFFER, 0, ibm_sz, ibms);
  glBufferSubData(GL_UNIFORM_BUFFER, ibm_sz, bone_sz, bones);
  glBufferSubData(GL_UNIFORM_BUFFER, bone_sz + ibm_sz, sizeof(int), &num_bones);
}
