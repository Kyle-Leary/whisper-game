#include "body.h"
#include "cglm/mat3.h"
#include "physics/constructor_macros.h"
#include "physics/physics.h"
#include "whisper/array.h"
#include <stdio.h>
#include <string.h>

// setup the common stuff shared between each body.
#define SETUP_TF(on_body)                                                      \
  memcpy(on_body.position, init_pos, sizeof(float) * 3);                       \
  memcpy(on_body.rotation, init_rotation, sizeof(float) * 4);                  \
  on_body.scale = init_scale;

RigidBody *make_rigid_body(float restitution, float position_lerp_speed,
                           float mass, float linear_damping,
                           float angular_damping, float static_friction,
                           float kinetic_friction, bool should_roll,
                           vec3 init_pos, float init_scale,
                           versor init_rotation) {
  // make a stack PhysComp, then copy that into the array.
  RigidBody body = {0};
  SETUP_TF(body)
  memcpy(body.lerp_position, body.position, sizeof(float) * 3);
  memcpy(body.ang_velocity, (vec3){0}, sizeof(float) * 3);
  memcpy(body.ang_acceleration, (vec3){0}, sizeof(float) * 3);

  body.restitution =
      restitution; // from 0 - 1, 1 is a perfectly elastic collision.

  body.position_lerp_speed = position_lerp_speed;

  body.mass = mass;
  body.linear_damping = linear_damping;
  body.angular_damping = angular_damping;

  body.should_roll = should_roll;

  // the caller can change this themselves on the object.
  body.frozen = false;

  body.static_friction = static_friction;
  body.kinetic_friction = kinetic_friction;

  // give a dummy initial value for the inverse inertia, just in case the caller
  // doesn't assign this properly.
  glm_mat3_identity(body.inverse_inertia);

  INDEX_AND_RETURN(body, rigid_bodies)
}

StaticBody *make_static_body(float restitution, vec3 init_pos, float init_scale,
                             versor init_rotation) {
  StaticBody sb;
  SETUP_TF(sb)
  sb.restitution = restitution;
  INDEX_AND_RETURN(sb, static_bodies)
}

AreaBody *make_area_body(vec3 init_pos, float init_scale,
                         versor init_rotation) {
  AreaBody ab;
  SETUP_TF(ab)
  INDEX_AND_RETURN(ab, area_bodies)
}

// we can resolve which types each body is generically here, and handle it all
// in one function.
void free_body(Body *b) {
  if (IS_RB(b)) {
    w_array_delete_ptr(&physics_state.rigid_bodies, b);
  } else if (IS_SB(b)) {
    w_array_delete_ptr(&physics_state.static_bodies, b);
  } else if (IS_AB(b)) {
    w_array_delete_ptr(&physics_state.area_bodies, b);
  }
}
