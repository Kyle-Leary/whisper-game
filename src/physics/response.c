#include "response.h"
#include "physics/body/body.h"
#include "physics/body/rigid_body.h"
#include "physics/component.h"
#include "physics/detection.h"
#include "physics/physics.h"
#include "util.h"
#include "whisper/array.h"
#include "whisper/queue.h"

#include <assert.h>
#include <stddef.h>

void response_pass() {
  // iter through all the physics components, and generically react to the
  // collision events inside the shapes.
  for (int i = 0; i < physics_state.phys_comps.upper_bound; i++) {
    PhysComp *comp = w_array_get(&(physics_state.phys_comps), i);

    Body *body = comp->body;
    if (((void *)body < physics_state.rigid_bodies_end) &&
        ((void *)body >= physics_state.rigid_bodies_start)) {
      // then body is a rigidbody
      RigidBody *rb = (RigidBody *)body;

      WQueue *mailbox = &(comp->collider->phys_events);
      WQueueSaveState state;
      w_queue_save_state(mailbox, &state);

      while (mailbox->active_elements > 0) {
        CollisionEvent *e = w_dequeue(mailbox);
        assert(
            e !=
            NULL); // shouldn't happen with the active elements condition above.

        rb_apply_force(rb, e->direction, e->magnitude, e->contact_pt);
      }

      w_queue_load_state(mailbox,
                         &state); // leave the data inside the queue untouched
                                  // for the object/owner of the actual physics
                                  // object to iterate through.
    }
  }
}
