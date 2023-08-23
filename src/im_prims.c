#include "im_prims.h"

#include "cglm/vec3.h"
#include "immediate.h"
#include "physics.h"

#define VEL_COLOR                                                              \
  (vec4) { 1, 0, 0, 1 }

#define ACCEL_COLOR                                                            \
  (vec4) { 0, 1, 0, 1 }

void im_velocity(PhysComp *phys) {
  vec3 line_positions[2];
  glm_vec3_copy(phys->position, line_positions[0]);
  glm_vec3_copy(phys->velocity, line_positions[1]);
  glm_vec3_add(line_positions[1], line_positions[0], line_positions[1]);
  im_draw((float *)line_positions, 2, VEL_COLOR, IM_LINE_STRIP);
}

void im_acceleration(PhysComp *phys) {
  vec3 line_positions[2];
  glm_vec3_copy(phys->position, line_positions[0]);
  glm_vec3_copy(phys->acceleration, line_positions[1]);
  glm_vec3_add(line_positions[1], line_positions[0], line_positions[1]);
  im_draw((float *)line_positions, 2, ACCEL_COLOR, IM_LINE_STRIP);
}
