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

void im_point(vec3 point) {
  im_draw((float *)point, 1, POINT_COLOR, IM_POINTS);
}
