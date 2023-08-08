#include "helper_math.h"

#include "cglm/affine.h"
#include "cglm/cglm.h"
#include "cglm/types.h"
#include "cglm/vec3.h"

#include <string.h>

void print_vec2(vec2 v) { printf("%f %f\n", v[0], v[1]); }
void print_vec3(vec3 v) { printf("%f %f %f\n", v[0], v[1], v[2]); }

void print_mat4(mat4 m) {
  // batching IO!!!
  // note: this is not actually IO batching. C will do IO batching like this and
  // only flush buffers either when they're full or there's a newline. (maybe
  // this is still effective syscall batching though? doesn't matter since it's
  // a debug function)
  printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", m[0][0],
         m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0],
         m[2][1], m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]);
}

Basis get_basis(mat4 m) {
  Basis b;
  memcpy(&b.right, &(vec3){m[0][0], m[1][0], m[2][0]}, sizeof(vec3));
  memcpy(&b.up, &(vec3){m[0][1], m[1][1], m[2][1]}, sizeof(vec3));
  memcpy(&b.forward, &(vec3){-m[0][2], -m[1][2], -m[2][2]}, sizeof(vec3));
  return b;
}

bool is_point_inside(AABB aabb, vec2 v) {
  return (v[0] >= aabb.xy[0] && v[0] <= (aabb.xy[0] + aabb.wh[0]) &&
          v[1] >= aabb.xy[1] && v[1] <= (aabb.xy[1] + aabb.wh[1]));
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
