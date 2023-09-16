#include "component.h"
#include "cglm/mat3.h"
#include "physics/body/body.h"
#include "physics/collider/collider.h"
#include "physics/constructor_macros.h"
#include "physics/physics.h"
#include "whisper/array.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// we need to calc inertia here, since we need to be aware of both the body and
// collider attached. we're putting the inertia tensor on the body, since the
// body's the one that's using it in collision response and dynamics.
PhysComp *make_physcomp(Body *body, Collider *collider) {
  // do the normal constructor stuff
  PhysComp comp;
  comp.body = body;
  comp.collider = collider;

  // then, link the collider's position with the body's position.
  comp.collider->body = body;

  if (IS_RB(body)) {
    // get the inverse inertia tensor from the collider.
    RigidBody *rb = (RigidBody *)body;
    mat3 inertia;
    glm_mat3_identity(inertia);

    if (IS_SPHERE(collider)) {
      SphereCollider *col = (SphereCollider *)collider;
      float mass = rb->mass;
      float radius =
          col->radius; // Assuming radius is defined in SphereCollider
      float inertia_value = (2.0f / 5.0f) * mass * radius * radius;
      glm_mat3_identity(inertia);
      glm_mat3_scale(inertia, inertia_value);
    } else if (IS_FLOOR(collider)) {
      // this should not happen.
      glm_mat3_scale(
          inertia,
          10000000000.0); // immovable on all rotation axes. recall that each
                          // diagonal on each axis of a diagonal matrix
                          // represents how hard the shape is to rotate on
                          // ONE AXIS. setting each diagonal to a really high
                          // number, therefore, prevents it from rotating.
                          // the inverse of the infinite inertia tensor is
                          // the zero matrix.
    } else if (IS_RECT(collider)) {
      RectCollider *col = (RectCollider *)collider;
      vec3 extents; // Assuming extents represents half-lengths along each axis
      memcpy(extents, col->extents, sizeof(float) * 3);
      float mass = rb->mass;

      inertia[0][0] =
          (1.0f / 12.0f) * mass *
          (4.0f * extents[1] * extents[1] + 4.0f * extents[2] * extents[2]);
      inertia[1][1] =
          (1.0f / 12.0f) * mass *
          (4.0f * extents[0] * extents[0] + 4.0f * extents[2] * extents[2]);
      inertia[2][2] =
          (1.0f / 12.0f) * mass *
          (4.0f * extents[0] * extents[0] + 4.0f * extents[1] * extents[1]);
    }

    // invert directly into the right memory slot on the rb.
    glm_mat3_inv(inertia, rb->inverse_inertia);
  }

  INDEX_AND_RETURN(comp, phys_comps)
}

void free_physcomp(PhysComp *physcomp) {
  free_body(physcomp->body);
  free_collider(physcomp->collider);
  w_array_delete_ptr(&physics_state.phys_comps, physcomp);
}
