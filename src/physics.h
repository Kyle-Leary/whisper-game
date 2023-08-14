#ifndef PHYSICS_H
#define PHYSICS_H
// for now, just a basic collision engine.

#include "cglm/types.h"
#include "object_bases.h"

#define GRAVITY_SCALE 0.1F

#include <stdint.h>

typedef struct PhysicsState {
} PhysicsState;

extern PhysicsState physics_state;

void physics_init();
// how we actually move objects in the world.
void physics_apply_force(PhysicsObject *po, vec3 force);
// the ticking function for this physics engine.
void physics_debug_draw();
void physics_update();
void physics_clean();

#endif
