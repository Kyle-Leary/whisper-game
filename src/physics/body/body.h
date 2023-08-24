#pragma once

#include "cglm/types.h"
#include <stdbool.h>

#define BODY_FIELDS vec3 position;

typedef struct Body {
  BODY_FIELDS
} Body;

typedef struct RigidBody {
  BODY_FIELDS

  // note that for objects with physics attached, the position is literally
  // stored inside of the physics component itself.
  vec3 lerp_position;
  float position_lerp_speed;
  vec3 velocity;
  vec3 acceleration;

  // represent the angle of the physics object with a unit quaternion
  versor angle;
  // speed of rotation around global coordinate axes.
  vec3 ang_velocity;
  vec3 ang_acceleration;

  float inertia;

  bool should_roll; // should the angle be manually calculated by the physics
                    // subsystem at all?

  float static_friction;
  float kinetic_friction;

  float mass;
  float linear_damping;

  bool frozen; // ignore all dynamics and forces applied to this rigid body.
} RigidBody;

typedef struct StaticBody {
  BODY_FIELDS
} StaticBody;

typedef struct AreaBody {
  BODY_FIELDS
} AreaBody;

RigidBody *make_rigid_body(float position_lerp_speed, float mass,
                           float linear_damping, float static_friction,
                           float kinetic_friction, bool should_roll,
                           vec3 init_pos);

StaticBody *make_static_body(vec3 init_pos);

AreaBody *make_area_body(vec3 init_pos);
