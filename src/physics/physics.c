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
  {
    w_make_array(&(physics_state.phys_comps), sizeof(PhysComp),
                 NUM_PHYS_COMPONENTS);
  }

  {
    {
      w_make_array(&(physics_state.rigid_bodies), sizeof(RigidBody),
                   MAX_RIGID_BODY);
      // instead of attaching a type to each body, we can determine which type
      // the data corresponds to with the location of the pointer, and whether
      // its in the array.
      physics_state.rigid_bodies_start = physics_state.rigid_bodies.buffer;
      physics_state.rigid_bodies_end =
          (char *)physics_state.rigid_bodies_start +
          (physics_state.rigid_bodies.full_elm_sz *
           physics_state.rigid_bodies.num_elms);

      w_make_array(&(physics_state.static_bodies), sizeof(StaticBody),
                   MAX_STATIC_BODY);
      w_make_array(&(physics_state.area_bodies), sizeof(AreaBody),
                   MAX_AREA_BODY);
    }

    {
      w_make_array(&(physics_state.rects), sizeof(RectCollider), MAX_RECTS);
      w_make_array(&(physics_state.spheres), sizeof(SphereCollider),
                   MAX_SPHERES);
      w_make_array(&(physics_state.floors), sizeof(FloorCollider), MAX_FLOORS);
    }
  }
}

void physics_update() {
  debug_shape_maintenance_pass();

  dynamics_pass();
  detection_pass();
  response_pass();
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
