#include "im_prims.h"

#include "cglm/types.h"
#include "cglm/vec3.h"
#include "immediate.h"

#define VEL_COLOR                                                              \
  (vec4) { 1, 0, 0, 1 }

#define ACCEL_COLOR                                                            \
  (vec4) { 0, 1, 0, 1 }

#define POINT_COLOR                                                            \
  (vec4) { 1, 1, 1, 1 }

void im_velocity(RigidBody *rb) {
  vec3 line_positions[2];
  glm_vec3_copy(rb->position, line_positions[0]);
  glm_vec3_copy(rb->velocity, line_positions[1]);
  glm_vec3_add(line_positions[1], line_positions[0], line_positions[1]);
  im_draw((float *)line_positions, 2, VEL_COLOR, IM_LINE_STRIP);
}

void im_acceleration(RigidBody *rb) {
  vec3 line_positions[2];
  glm_vec3_copy(rb->position, line_positions[0]);
  glm_vec3_copy(rb->acceleration, line_positions[1]);
  glm_vec3_add(line_positions[1], line_positions[0], line_positions[1]);
  im_draw((float *)line_positions, 2, ACCEL_COLOR, IM_LINE_STRIP);
}

void im_cube(vec3 center, float side_length) {
  // Half-lengths for convenience
  float half_side = side_length / 2.0f;

  // Define 8 corners of the cube
  vec3 corners[8];
  for (int i = 0; i < 8; ++i) {
    corners[i][0] = center[0] + ((i & 1) ? half_side : -half_side);
    corners[i][1] = center[1] + ((i & 2) ? half_side : -half_side);
    corners[i][2] = center[2] + ((i & 4) ? half_side : -half_side);
  }

  // Define vertices for 12 triangles (two per face)
  vec3 vertices[36];
  int indices[36] = {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 0, 1, 4, 4, 1, 5,
                     2, 3, 6, 6, 3, 7, 0, 2, 4, 4, 2, 6, 1, 3, 5, 5, 3, 7};

  for (int i = 0; i < 36; ++i) {
    glm_vec3_copy(corners[indices[i]], vertices[i]);
  }

  // Draw the cube using IM_TRIANGLES
  im_draw((float *)vertices, 36, (vec4){1, 1, 1, 1}, IM_TRIANGLES);
}

void im_point(vec3 point) { im_cube(point, 0.1); }
