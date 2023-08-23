#ifndef PHYSICS_H
#define PHYSICS_H
// for now, just a basic collision engine.

#include "cglm/types.h"
#include "event.h"
#include "object_bases.h"
#include "whisper/array.h"
#include "whisper/queue.h"

#include <stdint.h>

#define GRAVITY_SCALE 1.5F

// how many events can be stored by the physics component until it's
// overwritten?
#define PHYS_EVENT_QUEUE_SZ 8

// store all the physics components in each object in a list instead of inlined
// into their own seperate objects.
typedef struct PhysComp {
  // note that for objects with physics attached, the position is literally
  // stored inside of the physics component itself.
  vec3 lerp_position;
  float position_lerp_speed;
  vec3 position;
  vec3 velocity;
  vec3 acceleration;

  // represent the angle of the physics object with a unit quaternion
  versor angle;
  // speed of rotation around global coordinate axes.
  vec3 ang_velocity;
  vec3 ang_acceleration;

  bool should_roll; // should the angle be manually calculated by the physics
                    // subsystem at all?

  float static_friction;
  float kinetic_friction;

  float mass;
  float linear_damping;
  // NOTE: set phys_comp->colliders to NULL if the object has no collision
  // sender data/imposes no forces on other objects.
  Collider *colliders;
  unsigned int num_colliders;

  bool immovable; // immovable is a physics body property, being intangible is a
                  // collider property. colliders do the pushing, and bodies do
                  // the moving.
} PhysComp;

#define NUM_PHYS_COMPONENTS 200

typedef struct PhysicsState {
  WArray phys_comps;
} PhysicsState;

extern PhysicsState physics_state;

void physics_init();
// how we actually move objects in the world.
void physics_apply_force(PhysComp *comp, vec3 force, vec3 contact_pt);
// the ticking function for this physics engine.
void physics_debug_draw();
void physics_update();
void physics_clean();

// so that each caller doesn't have to do the "i'm moving the equal and opposite
// direction!" boilerplate, we'll make this generic over each physics comp, with
// the behavior controllable through the component parameters.
void react_physevent_generic(PhysComp *base_phys, PhysComp *target_phys,
                             PhysicsEvent *e);

// make a physcomp and allocate space in the array for it. return an array
// element pointer, so the caller can make sure that their PhysComp is being
// properly shared and iterated over by the physics subsystem.
PhysComp *make_physcomp(float position_lerp_speed, float mass,
                        float linear_damping, float static_friction,
                        float kinetic_friction, bool should_roll,
                        bool immovable, Collider *colliders, int num_colliders,
                        vec3 init_pos);

// make n colliders from the buffer passed in.
void make_colliders(uint num_col, Collider *dest);

#endif
