#pragma once

// handle the construction and modification of collider structures themselves in
// this module.

#include "event.h"
#include "event_types.h"
#include "object_bases.h"
#include <stdbool.h>

#include "physics/body/body.h"
#include "whisper/queue.h"

// this field shares a pointer to a subset of a different body, containing stuff
// like the versor for rotation and the vec3 for position. NOTE: in the
// detection, we can't assume anything about the body type other than the common
// fields already directly in Body. no typepunning!
#define COLLIDER_FIELDS                                                        \
  Body *body;                                                                  \
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
  vec3 extents; // how far each edge is from the rect's center of mass.
                // the x, y and z extent in one vector.
} RectCollider;

typedef struct FloorCollider {
  COLLIDER_FIELDS
} FloorCollider;

SphereCollider *make_sphere_collider(float radius);
FloorCollider *make_floor_collider();
RectCollider *make_rect_collider(vec3 extents);

void free_collider(Collider *c);
