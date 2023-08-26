#include "response.h"
#include "cglm/mat3.h"
#include "cglm/vec3.h"
#include "physics/body/body.h"
#include "physics/body/rigid_body.h"
#include "physics/component.h"
#include "physics/detection.h"
#include "physics/physics.h"
#include "printers.h"
#include "util.h"
#include "whisper/array.h"
#include "whisper/queue.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

static void rb_velocity_at_point(RigidBody *rb, vec3 r, vec3 dest) {
  // Angular velocity contribution (cross product of angular velocity and r)
  vec3 angular_velocity_contrib;
  glm_vec3_cross(rb->ang_velocity, r, angular_velocity_contrib);

  // Sum of the linear and angular contributions into dest
  glm_vec3_add(rb->velocity, angular_velocity_contrib, dest);
}

// Applying an Impulse. instantaneous change in velocity, we skip going through
// acceleration.
// we want to modify the velocity directly, since a collision happens over an
// instantaneous time, and we don't want objects to clip into eachother.
// impulses are often used to push objects around in CR.
static void rb_apply_impulse(RigidBody *rb, vec3 impulse, vec3 contact_pt) {
  vec3 delta_velocity;
  glm_vec3_scale(impulse, 1.0f / rb->mass, delta_velocity);
  glm_vec3_add(rb->velocity, delta_velocity, rb->velocity);

  // Calculate torque
  vec3 r; // Vector from the center of mass to the contact point
  glm_vec3_sub(contact_pt, rb->position, r);
  vec3 torque;
  glm_vec3_cross(r, impulse, torque);

  // Calculate change in angular velocity
  vec3 delta_angular_velocity;
  glm_mat3_mulv(rb->inverse_inertia, torque, delta_angular_velocity);

  // Update angular velocity
  glm_vec3_add(rb->ang_velocity, delta_angular_velocity, rb->ang_velocity);
}

static float balance_restitutions(float e1, float e2) {
  // we estimate the total collision's restitution through the e1 and e2 rests
  // of the bodies.
  float e_balanced = fminf(e1, e2);
  return e_balanced;
}

static void rb_respond_to_rb_collision(RigidBody *rb1, RigidBody *rb2,
                                       CollisionEvent *e) {
  vec3 r1, r2;
  glm_vec3_sub(e->contact_pt, rb1->position, r1);
  glm_vec3_sub(e->contact_pt, rb2->position, r2);

  vec3 vr1, vr2;
  rb_velocity_at_point(rb1, r1, vr1);
  rb_velocity_at_point(rb2, r2, vr2);
  vec3 v_rel;
  glm_vec3_sub(vr1, vr2, v_rel);
  float vn = glm_vec3_dot(v_rel, e->direction);

  if (vn > 0.0f)
    return; // Objects are moving away from each other

  float rest_balanced =
      balance_restitutions(rb1->restitution, rb2->restitution);
  float inv_mass_sum = (1.0f / rb1->mass) + (1.0f / rb2->mass);

  vec3 w1_cross_r1, w2_cross_r2;
  glm_vec3_cross(rb1->ang_velocity, r1, w1_cross_r1);
  glm_vec3_cross(rb2->ang_velocity, r2, w2_cross_r2);
  vec3 cross_prod;
  glm_vec3_add(w1_cross_r1, w2_cross_r2, cross_prod);
  float inv_inertia_sum = glm_vec3_dot(e->direction, cross_prod);

  float j = -(1.0f + rest_balanced) * vn / (inv_mass_sum + inv_inertia_sum);
  vec3 impulse;
  glm_vec3_scale(e->direction, j, impulse);

  rb_apply_impulse(rb1, impulse, r1);
}

static void rb_respond_to_sb_collision(RigidBody *rb, StaticBody *sb,
                                       CollisionEvent *e) {
  vec3 r;
  glm_vec3_sub(e->contact_pt, rb->position, r);

  glm_vec3_normalize(e->direction);

  vec3 vr;
  rb_velocity_at_point(rb, r, vr);
  float vn = glm_vec3_dot(vr, e->direction);

  if (vn > 0.0f)
    return; // Object is moving away from static body

  float rest_balanced = balance_restitutions(rb->restitution, sb->restitution);

  vec3 w_cross_r;
  glm_vec3_cross(rb->ang_velocity, r, w_cross_r);
  float inv_inertia = glm_vec3_dot(e->direction, w_cross_r);

  float j = -(1.0f + rest_balanced) * vn / ((1.0f / rb->mass) + inv_inertia);

  j += (e->depth * 3.5);

  vec3 impulse;
  glm_vec3_scale(e->direction, j, impulse);
  rb_apply_impulse(rb, impulse, r);
}

void response_pass() {
  // iter through all the physics components, and generically react to the
  // collision events inside the shapes.
  for (int i = 0; i < physics_state.phys_comps.upper_bound; i++) {
    PhysComp *comp = w_array_get(&(physics_state.phys_comps), i);

    // rigidbodies are the only ones moving, therefore we really only need to
    // iter through rigidbodies, at least right now. we're not going to have a
    // situation where a staticbody needs to detect another staticbody in CR,
    // especially since that wouldn't even have an effect.
    Body *body = comp->body;
    if (IS_RB(body)) {
      RigidBody *rb = (RigidBody *)body;

      WQueue *mailbox = &(comp->collider->phys_events);
      WQueueSaveState state;
      w_queue_save_state(mailbox, &state);

      while (mailbox->active_elements > 0) {
        CollisionEvent *e = w_dequeue(mailbox);
#ifdef DEBUG
        assert(
            e !=
            NULL); // shouldn't happen with the active elements condition above.
        assert(e->from != NULL);
#endif /* ifdef DEBUG */

        if (IS_RB(e->from)) {
          RigidBody *from = (RigidBody *)e->from;
          rb_respond_to_rb_collision(rb, from, e);
        } else if (IS_SB(e->from)) {
          StaticBody *from = (StaticBody *)e->from;
          rb_respond_to_sb_collision(rb, from, e);
        } else if (IS_AB(e->from)) {
          // do nothing.
        }
      }

      w_queue_load_state(mailbox,
                         &state); // leave the data inside the queue untouched
                                  // for the object/owner of the actual physics
                                  // object to iterate through.
    }
  }
}
