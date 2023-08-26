#pragma once

#include "cglm/types.h"
#include <stdalign.h>
#include <stdbool.h>

// store a generic transform, the collisions need this and every body should
// have at least this, or else it isn't much of a "physics body".
#define BODY_FIELDS                                                            \
  vec3 position;                                                               \
  float scale;                                                                 \
  versor rotation;

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
  // speed of rotation around global coordinate axes.
  vec3 ang_velocity;
  vec3 ang_acceleration;

  mat3 inverse_inertia; // we only use the inverse of the inertia tensor for
                        // in-engine calculation. store this directly.

  bool should_roll; // should the angle be manually calculated by the physics
                    // subsystem at all?

  float static_friction;
  float kinetic_friction;

  float mass;
  float linear_damping;
  float angular_damping;

  float restitution;

  bool frozen; // ignore all dynamics and forces applied to this rigid body.
} RigidBody;

typedef struct StaticBody {
  BODY_FIELDS

  float restitution;
} StaticBody;

typedef struct AreaBody {
  BODY_FIELDS
} AreaBody;

RigidBody *make_rigid_body(float restitution, float position_lerp_speed,
                           float mass, float linear_damping,
                           float angular_damping, float static_friction,
                           float kinetic_friction, bool should_roll,
                           vec3 init_pos, float init_scale,
                           versor init_rotation);

StaticBody *make_static_body(float restitution, vec3 init_pos, float init_scale,
                             versor init_rotation);

AreaBody *make_area_body(vec3 init_pos, float init_scale, versor init_rotation);
