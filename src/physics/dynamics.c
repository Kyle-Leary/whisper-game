#include "physics/body/rigid_body.h"
#include "physics/physics.h"
#include "whisper/array.h"

#include <stddef.h>

void dynamics_pass() {
  for (int i = 0; i < physics_state.rigid_bodies.upper_bound; i++) {
    RigidBody *rb = w_array_get(&(physics_state.rigid_bodies), i);
    if (rb != NULL) {
      single_rb_dynamics(rb);
    }
  }
}
