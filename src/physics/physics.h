#ifndef PHYSICS_H
#define PHYSICS_H
// for now, just a basic collision engine.

#include "cglm/types.h"
#include "event.h"
#include "object_bases.h"
#include "physics/body/body.h"
#include "physics/collider/collider.h"
#include "physics/component.h"
#include "whisper/array.h"
#include "whisper/queue.h"

#include <stdint.h>

#define GRAVITY_SCALE 0.3F

#define MAX_RIGID_BODY 16
#define MAX_STATIC_BODY 16
#define MAX_AREA_BODY 16
#define MAX_BODIES (MAX_RIGID_BODY + MAX_STATIC_BODY + MAX_AREA_BODY)

#define MAX_SPHERES 16
#define MAX_FLOORS 16
#define MAX_RECTS 16
#define MAX_COLLIDERS (MAX_SPHERES + MAX_FLOORS + MAX_RECTS)

#define NUM_PHYS_COMPONENTS (MIN(MAX_COLLIDERS, MAX_BODIES))

// wrangle all of the different arrays of bodies and shapes into one physics
// structure.
typedef struct PhysicsState {
  WArray phys_comps;

  WArray rigid_bodies;
  WArray static_bodies;
  WArray area_bodies;

  WArray floors;
  WArray spheres;
  WArray rects;

  // dont even ask
  void *rigid_bodies_start;
  void *rigid_bodies_end;
} PhysicsState;

extern PhysicsState physics_state;

void physics_init();
// the ticking function for this physics engine.
void physics_debug_draw();
void physics_update();
void physics_clean();

#endif
