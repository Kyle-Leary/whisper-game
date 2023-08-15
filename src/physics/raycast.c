#include "raycast.h"
#include "object.h"
#include "objects/floor.h"
#include "physics/collider_types.h"

// very similar to the physics update loop, for now. how can we only do the work
// of pulling out all the colliders once?
int raycast_intersect(uint16_t *indices, int num_indices, vec3 origin,
                      vec3 direction) {
  // only try to intersect with physics objects.
  PhysicsObject *phys_objects[NUM_OBJECTS];
  unsigned int n_phys_objects = 0;

  {
    for (int i = 0; i < NUM_OBJECTS; i++) {
      PhysicsObject *obj = (PhysicsObject *)object_state.objects[i];
      if (obj && IS_PHYS_OBJECT(obj->type) && obj->colliders) {
        phys_objects[n_phys_objects] = obj;
        n_phys_objects++;
      }
    }
  }

  int our_num_indices = 0;

  {
    for (int i = 0; i < n_phys_objects; i++) {
      PhysicsObject *target_obj = phys_objects[i];
      // this macro by default operates on the current target object's id,
      // pushing it into the list and then goto'ing out of the current
      // iteration, since the object's already been found. it also handles
      // hitting the index limit specified by the caller.
#define PUSH_INTO                                                              \
  {                                                                            \
    indices[our_num_indices] = target_obj->id;                                 \
    our_num_indices++;                                                         \
    if (our_num_indices >= num_indices) {                                      \
      return our_num_indices;                                                  \
    } else {                                                                   \
      goto next_object;                                                        \
    }                                                                          \
  }

      for (int col_i = 0; col_i < target_obj->num_colliders; col_i++) {
        Collider target_collider = target_obj->colliders[col_i];

        switch (target_collider.type) {

        case CL_FLOOR: {
          if (origin[1] < target_obj->position[1]) {
            // we're below the floor and colliding with the object by default.
            PUSH_INTO
          } else {
            // then we're above the floor, and the raycast is only colliding if
            // it's pointing down.
            if (direction[1] < 0) {
              PUSH_INTO
            }
          }
        } break;

        case CL_PILLAR: {
        } break;

        case CL_SPHERE: {
        } break;

        default:
          break;
        }
      }

    next_object:

#undef PUSH_INTO
    }

    // we didn't hit the caller index limit, return what we have.
    return our_num_indices;
  }
}
