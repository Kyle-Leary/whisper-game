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
#include "physics/collider.h"
#include "physics/collider_types.h"
#include "whisper/array.h"
#include "whisper/queue.h"

#include <bits/time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

PhysicsState physics_state = {}; // dummy for now.

// take in a root position, different colliders can be a specified offset from
// the root position of the physics component.
PhysComp *make_physcomp(float position_lerp_speed, float mass,
                        float linear_damping, float static_friction,
                        float kinetic_friction, bool should_roll,
                        bool immovable, Collider *colliders, int num_colliders,
                        vec3 init_pos) {
  // make a stack PhysComp, then copy that into the array.
  PhysComp p = {0};

  { // setup position.
    memcpy(p.position, init_pos, sizeof(float) * 3);
    memcpy(p.lerp_position, p.position, sizeof(float) * 3);
  }

  { // setup angle
    // the unit quaternion of no effect/rotation.
    memcpy(p.angle, (versor){0, 0, 0, 1}, sizeof(float) * 4);
    memcpy(p.ang_velocity, (vec3){0}, sizeof(float) * 3);
    memcpy(p.ang_acceleration, (vec3){0}, sizeof(float) * 3);
  }

  p.position_lerp_speed = position_lerp_speed;

  p.mass = mass;
  p.linear_damping = linear_damping;

  p.immovable = immovable;

  p.num_colliders = num_colliders;
  p.colliders = colliders;

  if (colliders != NULL) {
    // do init on all the colliders internally.
    for (int i = 0; i < num_colliders; i++) {
      Collider *c = &(colliders[i]);

      // calculate the inertia of the collider shape.
      switch (c->type) {
      case CL_SPHERE: {
        SphereColliderData *d = c->data;
        float radius = d->radius;

        c->inertia = ((2.0 / 5.0) * p.mass * powf(radius, 2));
      } break;

      default: {
        // ang. accel = torque / I, so if it's 0 we'll get an error.

        // make it really high, so that the default behavior is no rotation at
        // all.
        c->inertia = 999999999.0;
      } break;
      }
    }
  }

  p.static_friction = static_friction;
  p.kinetic_friction = kinetic_friction;

  int idx = w_array_insert(&(physics_state.phys_comps), &p);
  if (idx != -1) {
    // either return the element pointer or exit.
    return w_array_get(&(physics_state.phys_comps), idx);
  } else {
    fprintf(
        stderr,
        "Error: Could not allocate another physics component in the array.\n");
    exit(1);
  }
}

void make_colliders(uint num_col, Collider *dest) {
  for (int i = 0; i < num_col; i++) {
    w_make_queue(&(dest[i].phys_events), sizeof(PhysicsEvent),
                 PHYS_EVENT_QUEUE_SZ);
  }
}

void physics_init() {
  w_make_array(&(physics_state.phys_comps), sizeof(PhysComp),
               NUM_PHYS_COMPONENTS);
}

// a physics collision can be described as a force vector and contact point.
// we need a contact point for things like torque, which depend on knowing the
// force's distance from the center of mass.
void physics_apply_force(PhysComp *comp, vec3 force, vec3 contact_pt) {
  if (comp->immovable) {
    return;
  }

  // F = ma <=> F/m = a
  if (comp->mass == 0) {
    fprintf(
        stderr,
        "ERR: division by zero in force application since comp->mass is zero "
        "on object %p. Offsetting the mass slightly...\n",
        comp);
    comp->mass = 0.001F; // lol
  }

  glm_vec3_scale(force, (1 / comp->mass), force);
  glm_vec3_add(force, comp->acceleration, comp->acceleration);
}

void react_physevent_generic(PhysComp *base_phys, PhysComp *target_phys,
                             PhysicsEvent *e) {
  if (glm_vec3_distance((vec3){0}, e->normal) > 0.0001F) {
    glm_normalize(e->normal);
  }

  { // here, react generically to the collision event generated on the
    // target object.
    vec3 t_phys_force;
    // just apply the normalized force. the collision events are still
    // important, but for other things.
    glm_vec3_copy(e->normal, t_phys_force);
    glm_vec3_scale(t_phys_force, e->magnitude, t_phys_force);

    // apply the event's force to the target generically.
    physics_apply_force((PhysComp *)target_phys, t_phys_force, e->contact_pt);
    // then, apply the equal and opposite force to the sender.
    glm_vec3_scale(t_phys_force, -1, t_phys_force);
    physics_apply_force((PhysComp *)base_phys, t_phys_force, e->contact_pt);
  }
}

// apply various central forces that all affect the object about its center of
// mass.
static void apply_etc_forces(PhysComp *po) {
  vec3 net_force;
  glm_vec3_zero(net_force); // starts at no change in acceleration.

  { // gravity
    vec3 grav_force = {0, -1, 0};

    // F = mg
    glm_vec3_scale(grav_force, po->mass * GRAVITY_SCALE, grav_force);
    glm_vec3_add(net_force, grav_force, net_force);
  }

  { // damping
    vec3 damping_force;
    // linear damping is a force applied in the opposite direction of the
    // VELOCITY vector on the physics object.
    glm_vec3_scale(po->velocity, -po->linear_damping, damping_force);
    glm_vec3_add(net_force, damping_force, net_force);
  }

  {
#define GETRAND (((float)rand() / RAND_MAX) - 0.5)
    vec3 random = {GETRAND, GETRAND, GETRAND};
    glm_vec3_scale(random, 0.05, random);

#undef GETRAND
    glm_vec3_add(net_force, random, net_force);
  }

  physics_apply_force(po, net_force, po->position);

  glm_vec3_scale(po->acceleration, 0.85F, po->acceleration);
}

static void rk4_position(PhysComp *po) {
  vec3 k1_vel, k2_vel, k3_vel, k4_vel;
  vec3 k1_pos, k2_pos, k3_pos, k4_pos;
  vec3 tmp_vel, tmp_accel;

  // K1
  glm_vec3_scale(po->acceleration, delta_time, k1_vel);
  glm_vec3_scale(po->velocity, delta_time, k1_pos);

  // K2
  glm_vec3_scale(k1_vel, 0.5f, tmp_vel);
  glm_vec3_add(po->velocity, tmp_vel, tmp_vel);
  glm_vec3_scale(k1_vel, 0.5f, tmp_accel);
  glm_vec3_add(po->acceleration, tmp_accel, tmp_accel);
  glm_vec3_scale(tmp_accel, delta_time, k2_vel);
  glm_vec3_scale(tmp_vel, delta_time, k2_pos);

  // K3
  glm_vec3_scale(k2_vel, 0.5f, tmp_vel);
  glm_vec3_add(po->velocity, tmp_vel, tmp_vel);
  glm_vec3_scale(k2_vel, 0.5f, tmp_accel);
  glm_vec3_add(po->acceleration, tmp_accel, tmp_accel);
  glm_vec3_scale(tmp_accel, delta_time, k3_vel);
  glm_vec3_scale(tmp_vel, delta_time, k3_pos);

  // K4
  glm_vec3_add(po->velocity, k3_vel, tmp_vel);
  glm_vec3_add(po->acceleration, k3_vel, tmp_accel);
  glm_vec3_scale(tmp_accel, delta_time, k4_vel);
  glm_vec3_scale(tmp_vel, delta_time, k4_pos);

  // Combine
  for (int i = 0; i < 3; ++i) {
    po->velocity[i] +=
        (k1_vel[i] + 2 * k2_vel[i] + 2 * k3_vel[i] + k4_vel[i]) / 6.0f;
    po->position[i] +=
        (k1_pos[i] + 2 * k2_pos[i] + 2 * k3_pos[i] + k4_pos[i]) / 6.0f;
  }
}

static void position_lerp(PhysComp *po) {
  lerp_vec3(po->lerp_position, po->position, po->position_lerp_speed,
            po->lerp_position);
}

// define a hashtable over a bunch of pointers, the pointers to the collision
// objects. we want to map each collision object to one GraphicsRender*, for the
// purposes of reusing the same graphical representation VAOs.
#define DEBUG_SHAPE_HASHTABLE_LEN 2048

static GraphicsRender *shape_renders[DEBUG_SHAPE_HASHTABLE_LEN] = {0};

static unsigned int pointer_hash(void *key) {
  // this is good enough for now? i guess?
  return (unsigned long)key % DEBUG_SHAPE_HASHTABLE_LEN;
}

// debug draw ONE physics object.
static void physics_debug_shape_generate(PhysComp *po) {
  for (int i = 0; i < po->num_colliders; i++) {
    Collider *base_collider = &(po->colliders[i]);
    if (base_collider == NULL) {
      continue;
    }

    // debug rendering a collider is a property of the collider itself.
    if (base_collider->no_debug_render) {
      return;
    }

    int hashed = pointer_hash(base_collider);

    // first, check if the collider already has one associated with it.
    GraphicsRender *collider_render = shape_renders[hashed];

    if (collider_render) {
      { // maintain the debug render based on updated data. each and every
        // useful debug draw requires
        // frequent updates.

        { // position the render
          glm_mat4_identity(collider_render->model);
          glm_translate(collider_render->model, po->position);
        }

        switch (base_collider->type) {
        case CL_FLOOR: {
          // floor stretches infinitely far.
          glm_scale(collider_render->model, (vec3){50, 50, 50});
        } break;
        case CL_SPHERE: {
        } break;
        case CL_PILLAR: {
        } break;
        default: {
        } break;
        }
      }
    } else {
      // it's NULL and we need to make one.
      GraphicsRender *gr = NULL;

      switch (base_collider->type) {
      case CL_FLOOR: {
        FloorColliderData *data = (FloorColliderData *)base_collider->data;
        gr = glprim_floor_plane(po->position);
      } break;
      case CL_SPHERE: {
        SphereColliderData *data = (SphereColliderData *)base_collider->data;
        gr = glprim_sphere(po->position, data->radius, 8);
      } break;
      case CL_PILLAR: {
      } break;
      default: {
      } break;
      }

      if (gr) {
        // each physics debug shape renders in wireframe by default.
        gr->pc = PC_WIREFRAME;
        shape_renders[hashed] = gr;
      }
    }
  }
}

// draw all of the shapes.
void physics_debug_draw() {
  // lazy
  for (int i = 0; i < DEBUG_SHAPE_HASHTABLE_LEN; i++) {
    GraphicsRender *collider_render = shape_renders[i];

    if (collider_render) {
      glm_scale(collider_render->model, (vec3){1.1, 1.1, 1.1});
      g_draw_render(collider_render);
      glm_scale(collider_render->model, (vec3){0.9, 0.9, 0.9});
    }
  }
}

void physics_update() { // for now, this just runs
                        // through collision, and emits
  for (int i = 0; i < physics_state.phys_comps.upper_bound; i++) {
    PhysComp *base_phys = w_array_get(&(physics_state.phys_comps), i);
    if (base_phys == NULL) {
      continue;
    }

    physics_debug_shape_generate(base_phys);

    apply_etc_forces(base_phys);

    for (int j = 0; j < physics_state.phys_comps.upper_bound; j++) {
      if (i == j) // an object should not/need not collide with itself.
        continue;

      PhysComp *target_phys = w_array_get(&(physics_state.phys_comps), j);
      if (target_phys == NULL) {
        continue;
      }

      { /* generate the forces in the collision event from i -> j through a
           double iterator over both the collision arrays. */
        Collider *colliders = base_phys->colliders;
        if (colliders == NULL) {
          continue;
        }

        for (int col_i = 0; col_i < base_phys->num_colliders; col_i++) {
          Collider base_collider = colliders[col_i];

          if (base_collider.intangible) {
            // an intangible collider generates no forces on other colliders.
            // they're best used as detectors, since they can still take in
            // messages.
            continue;
          }

          // the collider of the BASE imposes itself on the collider of the
          // target. this is the BASE switch.
          switch (base_collider.type) {

          case CL_FLOOR: {
            handle_floor_collision(base_phys, base_collider, target_phys);
          } break;

          case CL_PILLAR: {
          } break;

          case CL_SPHERE: {
            handle_sphere_collision(base_phys, base_collider, target_phys);
          } break;

          default:
            break;
          }
        }
      }
    }

    rk4_position(
        base_phys); // update vel and position based on internal variables.
    position_lerp(base_phys); // update the lerp position based on the position.
  }
}

void physics_clean() {}
