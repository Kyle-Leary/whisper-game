#pragma once

// handle the construction and modification of collider structures themselves in
// this module.

#include "event.h"
#include "event_types.h"
#include "object_bases.h"
#include <stdbool.h>

#include "whisper/queue.h"

// this is weird, this field shares a pointer to the position of the physics
// body it's attached to. the body is the one that actually has position,
// and a shape doesn't make sense without a position.
#define COLLIDER_FIELDS                                                        \
  float *position;                                                             \
  WQueue phys_events;                                                          \
  bool inactive;

typedef struct Collider {
  COLLIDER_FIELDS
} Collider;

typedef struct SphereCollider {
  COLLIDER_FIELDS
  float radius;
} SphereCollider;

typedef struct RectCollider {
  COLLIDER_FIELDS
} RectCollider;

typedef struct FloorCollider {
  COLLIDER_FIELDS
} FloorCollider;

SphereCollider *make_sphere_collider(float radius);
FloorCollider *make_floor_collider();
RectCollider *make_rect_collider();
