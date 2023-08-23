#include "collider.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "event.h"
#include "global.h"
#include "physics.h"
#include "physics/collider_types.h"
#include "util.h"
#include "whisper/queue.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

/*
 * don't return anything. just modify and add in the force from the collision
 * reponse into the event pointer.
 * NOTE: this is a sphere base object colliding with something ELSE.
 * sphere -> target_phys, sphere imposes itself on another set of colliders from
 * object positional data.
 */
void handle_sphere_collision(PhysComp *base_phys, Collider base_collider,
                             PhysComp *target_phys) {
  assert(base_collider.type == CL_SPHERE);

  SphereColliderData *data = base_collider.data;
  float radius = data->radius;

  Collider *colliders = target_phys->colliders;
  if (colliders == NULL) {
    return; // they've opted out of collision.
  }

  for (int col_j = 0; col_j < target_phys->num_colliders; col_j++) {
    bool is_collision_detected = false;

    Collider *target_collider = &(colliders[col_j]);

    PhysicsEvent e; // event from col_i -> col_j, put in col_j's message queue
    e.sender_col_type = base_collider.type;

    switch (target_collider->type) {
    case CL_FLOOR: {
    } break;

    case CL_SPHERE: { // compare radii length.
      SphereColliderData *target_data = target_collider->data;
      float distance =
          glm_vec3_distance(base_phys->position, target_phys->position);
      if (distance < (radius + target_data->radius)) {
        // then the two spheres are intersecting.
        e.magnitude = 8 * distance;
        glm_vec3_sub(target_phys->position, base_phys->position,
                     e.normal); // base -> target force direction.

        is_collision_detected = true;
      }
    } break;

    case CL_PILLAR: {
    } break;

    default: {
    } break;
    }

    if (is_collision_detected) {
      w_enqueue(&(target_collider->phys_events), &e);

      if (!(target_collider->intangible)) {
        react_physevent_generic(base_phys, target_phys, &e);
      }
    }
  }
}

void handle_floor_collision(PhysComp *base_phys, Collider base_collider,
                            PhysComp *target_phys) {
  assert(base_collider.type == CL_FLOOR);

  Collider *colliders = target_phys->colliders;
  if (colliders == NULL) {
    return; // they've opted out of collision.
  }

  for (int col_j = 0; col_j < target_phys->num_colliders; col_j++) {
    bool is_collision_detected = false;

    Collider *target_collider = &(colliders[col_j]);

    PhysicsEvent e;
    e.sender_col_type = CL_FLOOR;

    switch (target_collider->type) {
    case CL_FLOOR: { // this is a dumb case, don't worry about it.
    } break;

    case CL_SPHERE: { // compare radii length.
      FloorColliderData *data = base_collider.data;
      SphereColliderData *target_data = target_collider->data;
      float difference = base_phys->position[1] - target_phys->position[1];

      float distance = target_data->radius - difference;
      if (distance <= 0) {
        vec3 normal_force;

        glm_vec3_scale((vec3){0, -1, 0}, -(GRAVITY_SCALE * target_phys->mass),
                       normal_force);
        glm_vec3_copy(normal_force, e.normal);
        e.magnitude = glm_vec3_distance((vec3){0}, normal_force);

        target_phys->velocity[1] = -(target_phys->velocity[1] / 5);

        is_collision_detected = true;
      }
    } break;

    case CL_PILLAR: { // a pillar is always colliding with the floor.
    } break;

    default: {
    } break;
    }

    if (is_collision_detected) {
      w_enqueue(&(target_collider->phys_events), &e);

      if (!(target_collider->intangible)) {
        react_physevent_generic(base_phys, target_phys, &e);
      }
    }
  }
}
