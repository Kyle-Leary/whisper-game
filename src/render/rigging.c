#include "rigging.h"
#include "shaders/ubo.h"

void rig_use_bones(mat4 *bones, mat4 *ibms, int num_bones) {
  ubo_push_bones(bones, ibms, num_bones);
}
