#include "helper_math.h"

#include "cglm/affine.h"
#include "cglm/cglm.h"
#include "cglm/types.h"
#include "cglm/vec3.h"

#include <string.h>

Basis get_basis(mat4 m) {
  Basis b;
  memcpy(&b.right, &(vec3){m[0][0], m[1][0], m[2][0]}, sizeof(vec3));
  memcpy(&b.up, &(vec3){m[0][1], m[1][1], m[2][1]}, sizeof(vec3));
  memcpy(&b.forward, &(vec3){-m[0][2], -m[1][2], -m[2][2]}, sizeof(vec3));
  return b;
}

bool is_point_inside(AABB aabb, vec2 v) {
  return (v[0] >= (aabb.center[0] - aabb.extents[0]) &&
          v[0] <= (aabb.center[0] + aabb.extents[0]) &&
          v[1] >= (aabb.center[1] - aabb.extents[1]) &&
          v[1] <= (aabb.center[1] + aabb.extents[1]));
}

float lerp(float a, float b, float t) { return (1 - t) * a + t * b; }

// lerping a -> b, add a by t * (b - a)
// lerping is the same equation, even for vectors.
void lerp_vec3(vec3 a, vec3 b, float t, vec3 dest) {
  // (b * t) + (a * (1 - t))
  vec3 temp_b, temp_a;
  glm_vec3_scale(b, t, temp_b);
  glm_vec3_scale(a, 1 - t, temp_a);
  glm_vec3_add(temp_a, temp_b, dest);
}

void mat4_lookat_point(mat4 mat, vec3 target) {
  vec3 eye;
  glm_mat4_pick3(mat, &eye);
  vec3 up = {0.0f, 1.0f, 0.0f}; // Assuming up direction is positive y-axis

  glm_lookat(eye, target, up,
             mat); // load the lookat matrix into the passed mat.
}
