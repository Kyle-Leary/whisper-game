#include "collider.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "event.h"
#include "physics/constructor_macros.h"
#include "physics/detection.h"
#include "physics/physics.h"
#include "util.h"
#include "whisper/array.h"
#include "whisper/queue.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

// how many events can be stored by the physics component until it's
// overwritten?
#define PHYS_EVENT_QUEUE_SZ 8

// generically setup the queue on the collider from a pointer, so that any of
// the other collider types can use this.
static void setup_collider(Collider *c) {
  w_make_queue(&(c->phys_events), sizeof(CollisionEvent), PHYS_EVENT_QUEUE_SZ);
}

SphereCollider *make_sphere_collider(float radius) {
  SphereCollider c;
  c.radius = radius;
  setup_collider((Collider *)&c);
  INDEX_AND_RETURN(c, spheres)
}

FloorCollider *make_floor_collider() {
  FloorCollider c;
  setup_collider((Collider *)&c);
  INDEX_AND_RETURN(c, floors)
}

RectCollider *make_rect_collider() {
  RectCollider c;
  setup_collider((Collider *)&c);
  INDEX_AND_RETURN(c, rects)
}
