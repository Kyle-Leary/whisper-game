#include "physics.h"
#include "backends/graphics_api.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "object.h"
#include "object_bases.h"
#include "object_lut.h"
#include "physics/collider.h"
#include "physics/collider_types.h"

#include <bits/time.h>
#include <string.h>
#include <time.h>

PhysicsState physics_state = {}; // dummy for now.

void physics_init() {}

void physics_apply_force(PhysicsObject *po, vec3 force) {
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

// right now: just damping and grav?
static void apply_etc_forces(PhysicsObject *po) {
  vec3 net_force;
  glm_vec3_zero(net_force); // starts at no change in acceleration.

  { // gravity
    vec3 grav_force = {0, -1, 0};

    // F = mg
    glm_vec3_scale(grav_force, po->mass * GRAVITY_SCALE, grav_force);
    glm_vec3_add(po->acceleration, grav_force, po->acceleration);
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

  glm_vec3_scale(po->acceleration, 0.8F, po->acceleration);
}

// get vel from accel and pos from vel.
static void euler_position(PhysicsObject *po) {
  vec3 tmp;
  glm_vec3_scale(po->acceleration, delta_time, tmp);
  glm_vec3_add(tmp, po->velocity, po->velocity);
  glm_vec3_scale(po->velocity, delta_time, tmp);
  glm_vec3_add(tmp, po->position, po->position);
}

static void position_lerp(PhysicsObject *po) {
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
static void physics_debug_shape_generate(PhysicsObject *po) {
  for (int i = 0; i < po->num_colliders; i++) {
    Collider *base_collider = &(po->colliders[i]);
    int hashed = pointer_hash(base_collider);

    // first, check if the collider already has one associated with it.
    GraphicsRender *collider_render = shape_renders[hashed];

    if (collider_render) {
      // maintain the render, drawing it happens in another function called
      // directly by the drawing loop in main().
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

      continue;
    }
    // else, create the render THEN insert it into the array.

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
      gr->pc = PC_WIREFRAME;
      shape_renders[hashed] = gr;
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
  // messages to the proper gameobjects.
  // a list of ptrs that we can safely operate on as physics objects.
  PhysicsObject *phys_objects[NUM_OBJECTS];
  unsigned int n_phys_objects = 0;

  { /* first, pull out the list of valid physics objects to narrow the
      double-iteration. in this scope, we modify the phys_objects stack data */

    for (int i = 0; i < NUM_OBJECTS; i++) {
      PhysicsObject *obj = (PhysicsObject *)object_state.objects[i];
      if (obj && IS_PHYS_OBJECT(obj->type) && obj->colliders) {
        phys_objects[n_phys_objects] = obj;
        n_phys_objects++;
      }
    }
  }

  { // then, handle all the force generation stuff in this scope on EVERY
    // physobject.
    for (int i = 0; i < n_phys_objects; i++) {
      PhysicsObject *base_obj = phys_objects[i];

      physics_debug_shape_generate(base_obj);

      apply_etc_forces(base_obj);

      for (int j = 0; j < n_phys_objects; j++) {
        if (i == j) // an object should not/need not collide with itself.
          continue;
        PhysicsObject *target_obj = phys_objects[j];

        // both i and j objects are valid collision objects, and different
        // ones, so check the force and generate an event from i -> j.
        CollisionEvent *e = (CollisionEvent *)calloc(sizeof(CollisionEvent), 1);
        // else, the object is valid and has a valid collision array.

        memcpy(e->normalized_force, (vec3){0, 0, 0}, sizeof(vec3));
        e->magnitude = 0.00F; // empty force, doesn't move anything.

        e->id = i; // use the entity id as the id for collisions.

        { /* generate the forces in the collision event from i -> j through a
             double iterator over both the collision arrays. */
          for (int col_i = 0; col_i < base_obj->num_colliders; col_i++) {
            Collider base_collider = base_obj->colliders[col_i];

            // the collider of the BASE imposes itself on the collider of the
            // target. this is the BASE switch.
            switch (base_collider.type) {

            case CL_FLOOR: {
              FloorColliderData *data = base_collider.data;
              float difference =
                  base_obj->position[1] - target_obj->position[1];

              // TODO: apply friction when the object is just in the range of
              // the floor.

              if (difference > 0) {
                // if the floor is above the target, then push it up. push it up
                // with a normal just as strong as the acceleration of the body
                // pushing into it, just the opposite way. this is the "ideal"
                // floor, which will never break.
                vec3 normal_force;
                // TODO: properly calculate the normal force of the object on
                // the floor.
                glm_vec3_scale((vec3){0, -1, 0},
                               -(GRAVITY_SCALE * target_obj->mass),
                               normal_force);
                glm_vec3_copy(normal_force, e->normalized_force);
                e->magnitude = glm_vec3_distance((vec3){0}, normal_force);

                target_obj->velocity[1] = 0;
                target_obj->position[1] = base_obj->position[1];
              }
            } break;

            case CL_PILLAR: {
            } break;

            case CL_SPHERE: {
              handle_sphere_collision(base_obj, base_collider, target_obj, e);
            } break;

            default:
              break;
            }
          }
        }

        if (glm_vec3_distance((vec3){0}, e->normalized_force) > 0.0001F) {
          glm_normalize(e->normalized_force);
        }

        /* then, send the batched collision response over the function pointer
         to the target object, allowing them to react however they will. */
        fn_lut[target_obj->type].col_handler((void *)target_obj, e);

        { // here, react generically to the collision event generated on the
          // target object.
          vec3 t_obj_force;
          // just apply the normalized force. the collision events are still
          // important, but for other things.
          glm_vec3_copy(e->normalized_force, t_obj_force);
          glm_vec3_scale(t_obj_force, e->magnitude, t_obj_force);

          // apply the event's force to the target generically.
          physics_apply_force((PhysicsObject *)target_obj, t_obj_force);
          // then, apply the equal and opposite force to the sender.
          glm_vec3_scale(t_obj_force, -1, t_obj_force);
          physics_apply_force((PhysicsObject *)base_obj, t_obj_force);
        }
      }

      euler_position(
          base_obj); // update vel and position based on internal variables.
      position_lerp(
          base_obj); // update the lerp position based on the position.
    }
  }
}

void physics_clean() {}
