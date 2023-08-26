#include "transform.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "printers.h"
#include "util.h"

void m4_apply_transform(mat4 m4, vec3 position, float scale, versor rotation) {
  glm_mat4_identity(m4);

  // translate
  glm_translate(m4, position);

  // rotate
  // make the rotation matrix from the versor
  mat4 rot_mat;
  glm_quat_mat4(rotation, rot_mat);
  glm_mat4_mul(m4, rot_mat, m4);

  // scale
  glm_mat4_scale(m4, scale);
}

void m4_apply_transform_from_body(mat4 m4, Body *b) {
  m4_apply_transform(m4, b->position, b->scale, b->rotation);
}
