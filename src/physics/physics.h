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

#define GRAVITY_SCALE 1.0F

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
  float accumulator;         // the internal state counting up the delta time.
  float accumulator_trigger; // when does the accumulator trigger another
                             // physics update?
  float accumulator_clamp_max;

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
  void *static_bodies_start;
  void *static_bodies_end;
  void *area_bodies_start;
  void *area_bodies_end;

  void *floors_start;
  void *floors_end;
  void *spheres_start;
  void *spheres_end;
  void *rects_start;
  void *rects_end;
} PhysicsState;

extern PhysicsState physics_state;

// the consistent global fixed timestep.
#define DT (physics_state.accumulator_trigger)

#define CHECK_IN(array, obj)                                                   \
  (((void *)obj < physics_state.array##_end) &&                                \
   ((void *)obj >= physics_state.array##_start))

#define IS_RB(body) CHECK_IN(rigid_bodies, body)
#define IS_SB(body) CHECK_IN(static_bodies, body)
#define IS_AB(body) CHECK_IN(area_bodies, body)

#define IS_FLOOR(col) CHECK_IN(floors, col)
#define IS_SPHERE(col) CHECK_IN(spheres, col)
#define IS_RECT(col) CHECK_IN(rects, col)

void physics_init();
// the ticking function for this physics engine.
void physics_debug_draw();
void physics_update();
void physics_clean();

#endif
