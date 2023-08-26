#include "physics.h"
#include "backends/graphics_api.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "event.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "object.h"
#include "object_bases.h"
#include "object_lut.h"
#include "physics/body/body.h"
#include "physics/collider/collider.h"
#include "physics/collider/shape_debug_draw.h"
#include "physics/detection.h"
#include "physics/dynamics.h"
#include "physics/response.h"
#include "whisper/array.h"
#include "whisper/queue.h"

#include <bits/time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

PhysicsState physics_state = {0};

// don't make sub-inits, that's just annoying to run around and try to read.
void physics_init() {
#define CALC_START_END(array)                                                  \
  {                                                                            \
    physics_state.array##_start = physics_state.array.buffer;                  \
    physics_state.array##_end =                                                \
        (char *)physics_state.array##_start +                                  \
        (physics_state.array.full_elm_sz * physics_state.array.num_elms);      \
  }

  {
    w_make_array(&(physics_state.phys_comps), sizeof(PhysComp),
                 NUM_PHYS_COMPONENTS);
  }

  {
    {
      w_make_array(&(physics_state.rigid_bodies), sizeof(RigidBody),
                   MAX_RIGID_BODY);
      CALC_START_END(rigid_bodies);
      // instead of attaching a type to each body, we can determine which type
      // the data corresponds to with the location of the pointer, and whether
      // its in the array.

      w_make_array(&(physics_state.static_bodies), sizeof(StaticBody),
                   MAX_STATIC_BODY);
      CALC_START_END(static_bodies);

      w_make_array(&(physics_state.area_bodies), sizeof(AreaBody),
                   MAX_AREA_BODY);
      CALC_START_END(area_bodies);
    }

    {
      w_make_array(&(physics_state.rects), sizeof(RectCollider), MAX_RECTS);
      CALC_START_END(rects);
      w_make_array(&(physics_state.spheres), sizeof(SphereCollider),
                   MAX_SPHERES);
      CALC_START_END(spheres);
      w_make_array(&(physics_state.floors), sizeof(FloorCollider), MAX_FLOORS);
      CALC_START_END(floors);
    }
  }
#undef CALC_START_END
}

void physics_update() {
  debug_shape_maintenance_pass();

  // generate all the collision point structures that we'll use to respond to
  // collisions. this should be primarily focused on the shapes at play, not the
  // bodies and response parameters.
  detection_pass();

  // make all the changes to the internal variables of the bodies. this should
  // use the collisionevents and the body parameters, and be mostly
  // shape-ambivalent.
  response_pass();

  // then, process all those changes internally to move the body. this should
  // only ever need to use the shape, dynamics is the simplest (data-wise) of
  // all three passes.
  dynamics_pass();
}

void physics_debug_draw() { debug_shape_draw(); }

void physics_clean() {
  w_clean_array(&(physics_state.phys_comps));
  w_clean_array(&(physics_state.rigid_bodies));
  w_clean_array(&(physics_state.static_bodies));
  w_clean_array(&(physics_state.area_bodies));
  w_clean_array(&(physics_state.rects));
  w_clean_array(&(physics_state.spheres));
  w_clean_array(&(physics_state.floors));
}
