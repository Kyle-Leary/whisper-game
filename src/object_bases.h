#pragma once

#include "cglm/types.h"
#include "physics/collider_types.h"

// keep the masks low, since our vtable allocation will be of OBJ_COUNT size.
#define PHYS_OBJ_MASK 0b1000000000

// to get the actual length of the enum, we'll keep track of the individual
// section lengths and do out the math in a macro.
typedef enum ObjectType {
  // world
  OBJ_RENDER,

  // hud
  OBJ_LABEL,
  OBJ_BUTTON,
  OBJ_TEXTURE,

  // we need to be able to quickly discern whether an object is a physics
  // object, use a masking type pattern again.
  OBJ_PLAYER = PHYS_OBJ_MASK,
  OBJ_CAMERA,

  // prims
  OBJ_CUBE,
  OBJ_SPHERE,
  OBJ_FLOOR,

  OBJ_AREAOBJ,
  OBJ_CHARACTER,

  OBJ_COUNT,
} ObjectType;

// scuffed
#define IS_PHYS_OBJECT(o_type) (o_type >= PHYS_OBJ_MASK)

// and now we can just use it like normal, kind of. instead of renewing a long
// at the most basic level, an object doesn't really have much.
#define OBJECT_FIELDS ObjectType type;

// handled in the external LUT, an object is linked to a FnPointers structure by
// its type.
typedef struct Object {
  OBJECT_FIELDS
  // NOTE: set colliders to NULL if the object has no collision sender
  // data/imposes no forces on other objects.
} Object;

// for an object to really impose physics, it needs to have a meaningful
// position. we can't be too generic with physics.
#define PHYS_OBJECT_FIELDS                                                     \
  OBJECT_FIELDS                                                                \
  vec3 lerp_position;                                                          \
  float position_lerp_speed;                                                   \
  vec3 position;                                                               \
  vec3 velocity;                                                               \
  vec3 acceleration;                                                           \
  int immovable;                                                               \
  float mass;                                                                  \
  float linear_damping;                                                        \
  Collider *colliders;                                                         \
  unsigned int num_colliders;

typedef struct PhysicsObject {
  PHYS_OBJECT_FIELDS
} PhysicsObject;
