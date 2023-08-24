#include "detection.h"
#include "cglm/quat.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "physics/collider/collider.h"
#include "physics/physics.h"
#include "util.h"
#include "whisper/array.h"
#include "whisper/queue.h"

#include <stddef.h>
#include <string.h>

#define DETECTION_DIFFERENT(type_one, type_two, array_one, array_two)          \
  for (int i = 0; i < physics_state.array_one.upper_bound; i++) {              \
    type_one *base = w_array_get(&physics_state.array_one, i);                 \
    if (base == NULL)                                                          \
      continue;                                                                \
    for (int j = 0; j < physics_state.array_two.upper_bound; j++) {            \
      type_two *target = w_array_get(&physics_state.array_two, j);             \
      if (target == NULL)                                                      \
        continue;                                                              \
      CollisionEvent e_into_target;                                            \
      CollisionEvent e_into_base;

// add one to the inner index to not generate collisions between the shape and
// itself.
#define DETECTION_SAME(type, array)                                            \
  for (int i = 0; i < physics_state.array.upper_bound; i++) {                  \
    type *base = w_array_get(&physics_state.array, i);                         \
    if (base == NULL)                                                          \
      continue;                                                                \
    for (int j = i + 1; j < physics_state.array.upper_bound; j++) {            \
      type *target = w_array_get(&physics_state.array, j);                     \
      if (target == NULL)                                                      \
        continue;                                                              \
      CollisionEvent e_into_target;                                            \
      CollisionEvent e_into_base;

#define DETECTION_PUSH                                                         \
  w_enqueue(&(target->phys_events), &e_into_target);                           \
  w_enqueue(&(base->phys_events), &e_into_base);

#define DETECTION_END                                                          \
  }                                                                            \
  }

void handle_sphere_sphere() {
  DETECTION_SAME(SphereCollider, spheres)

  float distance =
      glm_vec3_distance(base->body->position, target->body->position);
  if (distance < (base->radius + target->radius)) {
    float magnitude = distance;
    e_into_base.magnitude = magnitude;
    e_into_target.magnitude = magnitude;

    glm_vec3_sub(target->body->position, base->body->position,
                 e_into_target.direction);
    glm_vec3_scale(e_into_target.direction, -1, e_into_base.direction);

    DETECTION_PUSH
  }

  DETECTION_END
}

void handle_sphere_rect() {
  DETECTION_DIFFERENT(SphereCollider, RectCollider, spheres, rects)

  DETECTION_END
}

void handle_sphere_floor() {
  DETECTION_DIFFERENT(SphereCollider, FloorCollider, spheres, floors)

  float distance =
      target->body->position[1] - (base->body->position[1] - base->radius);
  if (distance >= 0) {
    float magnitude = distance * 8;
    e_into_base.magnitude = magnitude;
    e_into_target.magnitude = magnitude;

    // the sphere pushes DOWN into the floor
    glm_vec3_copy((vec3){0, -1, 0}, e_into_target.direction);
    // the floor gives an equal and opposite reaction
    glm_vec3_scale(e_into_target.direction, -1, e_into_base.direction);

    DETECTION_PUSH
  }

  DETECTION_END
}

void handle_rect_rect() {
  DETECTION_SAME(RectCollider, rects)

  DETECTION_END
}

void handle_rect_floor() {
  DETECTION_DIFFERENT(RectCollider, FloorCollider, rects, floors)

  vec3 extents;
  memcpy(&extents, &(base->extents), sizeof(float) * 3);
  vec3 positions[8];

  {
    memcpy(&(positions[0]), (vec3){-extents[0], -extents[1], extents[2]},
           sizeof(float) * 3); // Bottom left front
    memcpy(&(positions[1]), (vec3){extents[0], -extents[1], extents[2]},
           sizeof(float) * 3); // Bottom right front
    memcpy(&(positions[2]), (vec3){extents[0], extents[1], extents[2]},
           sizeof(float) * 3); // Top right front
    memcpy(&(positions[3]), (vec3){-extents[0], extents[1], extents[2]},
           sizeof(float) * 3); // Top left front
    memcpy(&(positions[4]), (vec3){-extents[0], -extents[1], -extents[2]},
           sizeof(float) * 3); // Bottom left back
    memcpy(&(positions[5]), (vec3){extents[0], -extents[1], -extents[2]},
           sizeof(float) * 3); // Bottom right back
    memcpy(&(positions[6]), (vec3){extents[0], extents[1], -extents[2]},
           sizeof(float) * 3); // Top right back
    memcpy(&(positions[7]), (vec3){-extents[0], extents[1], -extents[2]},
           sizeof(float) * 3); // Top left back
  }

  for (int k = 0; k < 8; k++) {
    // apply the unit versor rotation to the points
    glm_quat_rotatev(positions[0], base->body->rotation, positions[0]);
  }

  for (int k = 0; k < 8; k++) {
    vec3 curr_pt;
    glm_vec3_add(positions[k], base->body->position, curr_pt);

    float distance = target->body->position[1] - curr_pt[1];
    if (distance >= 0) {
      float magnitude = distance * 4;
      e_into_base.magnitude = magnitude;
      e_into_target.magnitude = magnitude;

      // the sphere pushes DOWN into the floor
      glm_vec3_copy((vec3){0, -1, 0}, e_into_target.direction);
      // the floor gives an equal and opposite reaction
      glm_vec3_scale(e_into_target.direction, -1, e_into_base.direction);

      DETECTION_PUSH
    }
  }

  DETECTION_END
}

// trivial case, this won't generate any collisions with our definition of
// "floor" here.
void handle_floor_floor();

#define CLEAN(array)                                                           \
  for (int i = 0; i < physics_state.array.upper_bound; i++) {                  \
    Collider *base = w_array_get(&physics_state.array, i);                     \
    WQueue *mailbox = &(base->phys_events);                                    \
    while (mailbox->active_elements > 0) {                                     \
      CollisionEvent *e = w_dequeue(mailbox);                                  \
    }                                                                          \
  }

void detection_pass() {
  CLEAN(spheres)
  CLEAN(floors)
  CLEAN(rects)

  handle_sphere_sphere();
  handle_sphere_rect();
  handle_sphere_floor();
  handle_rect_rect();
  handle_rect_floor();
}
