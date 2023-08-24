#include "body.h"
#include "physics/constructor_macros.h"
#include "physics/physics.h"
#include "whisper/array.h"
#include <stdio.h>
#include <string.h>

RigidBody *make_rigid_body(float position_lerp_speed, float mass,
                           float linear_damping, float angular_damping,
                           float static_friction, float kinetic_friction,
                           bool should_roll, vec3 init_pos) {
  // make a stack PhysComp, then copy that into the array.
  RigidBody body = {0};

  { // setup position.
    memcpy(body.position, init_pos, sizeof(float) * 3);
    memcpy(body.lerp_position, body.position, sizeof(float) * 3);
  }

  { // setup angle
    // the unit quaternion of no effect/rotation.
    memcpy(body.rotation, (versor){0, 0, 0, 1}, sizeof(float) * 4);
    memcpy(body.ang_velocity, (vec3){0}, sizeof(float) * 3);
    memcpy(body.ang_acceleration, (vec3){0}, sizeof(float) * 3);
  }

  body.position_lerp_speed = position_lerp_speed;

  body.mass = mass;
  body.linear_damping = linear_damping;
  body.angular_damping = angular_damping;

  body.should_roll = should_roll;

  // the caller can change this themselves on the object.
  body.frozen = false;

  body.static_friction = static_friction;
  body.kinetic_friction = kinetic_friction;

  INDEX_AND_RETURN(body, rigid_bodies)
}

StaticBody *make_static_body(vec3 init_pos) {
  StaticBody sb;
  memcpy(sb.position, init_pos, sizeof(float) * 3);
  INDEX_AND_RETURN(sb, static_bodies)
}

AreaBody *make_area_body(vec3 init_pos) {
  AreaBody ab;
  memcpy(ab.position, init_pos, sizeof(float) * 3);
  INDEX_AND_RETURN(ab, area_bodies)
}
