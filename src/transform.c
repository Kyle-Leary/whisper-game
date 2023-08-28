#include "transform.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "cglm/vec2.h"
#include "helper_math.h"
#include "printers.h"
#include "util.h"

void aabb_apply_transform(AABB *to, AABB *by, AABB *dest) {
  // Translating and scaling the child's center based on the parent's center and
  // extents
  dest->center[0] = by->center[0] + (to->center[0] - 0.5) * by->extents[0];
  dest->center[1] = by->center[1] + (to->center[1] - 0.5) * by->extents[1];

  // Scaling the child's extents based on the parent's extents
  dest->extents[0] = to->extents[0] * by->extents[0];
  dest->extents[1] = to->extents[1] * by->extents[1];
}

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
