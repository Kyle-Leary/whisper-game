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
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

PhysicsState physics_state = {}; // dummy for now.

PhysComp *make_physcomp(float position_lerp_speed, float mass,
                        float linear_damping, int immovable, int intangible,
                        Collider *colliders, int num_colliders, vec3 init_pos,
                        int no_debug_render) {
  // make a stack PhysComp, then copy that into the array.
  PhysComp p = {0};

  memcpy(p.position, init_pos, sizeof(float) * 3);
  memcpy(p.lerp_position, p.position, sizeof(float) * 3);

  p.position_lerp_speed = position_lerp_speed;

  p.intangible = intangible;
  p.immovable = immovable;

  p.mass = mass;
  p.linear_damping = linear_damping;

  p.num_colliders = num_colliders;
  p.colliders = colliders;

  p.no_debug_render = no_debug_render;

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
    w_make_queue(&(dest->phys_events), sizeof(PhysicsEvent),
                 PHYS_EVENT_QUEUE_SZ);
  }
}

void physics_init() {
  w_make_array(&(physics_state.phys_comps), sizeof(PhysComp),
               NUM_PHYS_COMPONENTS);
}

// this is the physics equivalent of a positional setter. just modify the
// acceleration, taking into account the PhysComp's mass and settings.
void physics_apply_force(PhysComp *po, vec3 force) {
  if (po->immovable) // immovable is 1, have to manually opt-in to immovability
                     // out of the calloc for the physobj.
    return;

  // F = ma <=> F/m = a
  if (po->mass == 0) {
    fprintf(stderr,
            "ERR: division by zero in force application since po->mass is zero "
            "on object %p. Offsetting the mass slightly...\n",
            po);
    po->mass = 0.001F; // lol
  }

  glm_vec3_scale(force, (1 / po->mass), force);
  glm_vec3_add(force, po->acceleration, po->acceleration);
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
    physics_apply_force((PhysComp *)target_phys, t_phys_force);
    // then, apply the equal and opposite force to the sender.
    glm_vec3_scale(t_phys_force, -1, t_phys_force);
    physics_apply_force((PhysComp *)base_phys, t_phys_force);
  }
}

// right now: just damping and grav?
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

  physics_apply_force(po, net_force);

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
  if (po->no_debug_render) {
    return;
  }

  for (int i = 0; i < po->num_colliders; i++) {
    Collider *base_collider = &(po->colliders[i]);
    if (base_collider == NULL) {
      continue;
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
        for (int col_i = 0; col_i < base_phys->num_colliders; col_i++) {
          Collider *colliders = base_phys->colliders;
          if (colliders == NULL) {
            continue;
          }

          Collider base_collider = colliders[col_i];

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
