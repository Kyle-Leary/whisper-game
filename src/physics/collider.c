#include "collider.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "physics.h"
#include <stdio.h>

/*
 * don't return anything. just modify and add in the force from the collision
 * reponse into the event pointer.
 * NOTE: this is a sphere base object colliding with something ELSE.
 * sphere -> target_obj, sphere imposes itself on another set of colliders from
 * object positional data.
 */
void handle_sphere_collision(PhysicsObject *base_obj, Collider base_collider,
                             PhysicsObject *target_obj, CollisionEvent *e) {
  SphereColliderData *data = base_collider.data;
  float radius = data->radius;

  for (int col_j = 0; col_j < target_obj->num_colliders; col_j++) {
    Collider target_collider = target_obj->colliders[col_j];
    switch (target_collider.type) {
    case CL_FLOOR: {
    } break;

    case CL_SPHERE: { // compare radii length.
      SphereColliderData *target_data = target_collider.data;
      float distance =
          glm_vec3_distance(base_obj->position, target_obj->position);
      if (distance < (radius + target_data->radius)) {
        // then the two spheres are intersecting.
        e->magnitude = 0.1F * distance;
        glm_vec3_sub(target_obj->position, base_obj->position,
                     e->normalized_force); // base -> target force direction.
      }
    } break;

    case CL_PILLAR: {
    } break;

    default: {
    } break;
    }
  }
}
