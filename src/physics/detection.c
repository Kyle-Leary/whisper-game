#include "detection.h"
#include "cglm/vec3.h"
#include "physics/collider/collider.h"
#include "physics/physics.h"
#include "util.h"
#include "whisper/array.h"

#include <stddef.h>

#define DETECTION_INNER(type_one, type_two, array_one, array_two, mod)         \
  for (int i = 0; i < physics_state.array_one.upper_bound; i++) {              \
    type_one *base = w_array_get(&physics_state.array_one, i);                 \
    if (base == NULL)                                                          \
      continue;                                                                \
    for (int j = i mod; j < physics_state.array_two.upper_bound; j++) {        \
      CollisionEvent e_into_target;                                            \
      CollisionEvent e_into_base;                                              \
      bool is_collision_detected = false;                                      \
      type_two *target = w_array_get(&physics_state.array_two, j);             \
      if (target == NULL)                                                      \
        continue;

// intro has no modifier on the index, since they're not modifying the same
// array.
#define DETECTION_DIFFERENT(type_one, type_two, array_one, array_two)          \
  DETECTION_INNER(type_one, type_two, array_one, array_two, )

// add one to the inner index to not generate collisions between the shape and
// itself.
#define DETECTION_SAME(type, array)                                            \
  DETECTION_INNER(type, type, array, array, +1)

#define DETECTION_END                                                          \
  if (is_collision_detected) {                                                 \
    w_enqueue(&(target->phys_events), &e_into_target);                         \
    w_enqueue(&(base->phys_events), &e_into_base);                             \
  }                                                                            \
  }                                                                            \
  }

void handle_sphere_sphere() {
  DETECTION_SAME(SphereCollider, spheres)

  float distance = glm_vec3_distance(base->position, target->position);
  if (distance < (base->radius + target->radius)) {
    float magnitude = 8 * distance;
    e_into_base.magnitude = magnitude;
    e_into_target.magnitude = magnitude;

    glm_vec3_sub(target->position, base->position, e_into_target.direction);
    glm_vec3_scale(e_into_target.direction, -1, e_into_base.direction);

    is_collision_detected = true;
  }

  DETECTION_END
}

void handle_sphere_rect() {
  DETECTION_DIFFERENT(SphereCollider, RectCollider, spheres, rects)

  DETECTION_END
}

void handle_sphere_floor() {
  DETECTION_DIFFERENT(SphereCollider, FloorCollider, spheres, floors)

  float distance = target->position[1] - base->position[1];
  if (distance >= 0) {
    float magnitude = 20 * distance;
    e_into_base.magnitude = magnitude;
    e_into_target.magnitude = magnitude;

    // the sphere pushes DOWN into the floor
    glm_vec3_copy((vec3){0, -1, 0}, e_into_target.direction);
    // the floor gives an equal and opposite reaction
    glm_vec3_scale(e_into_target.direction, -1, e_into_base.direction);

    is_collision_detected = true;
  }

  DETECTION_END
}

void handle_rect_rect() {
  DETECTION_SAME(RectCollider, rects)

  DETECTION_END
}

void handle_rect_floor() {
  DETECTION_DIFFERENT(RectCollider, FloorCollider, rects, floors)

  DETECTION_END
}

// trivial case, this won't generate any collisions with our definition of
// "floor" here.
void handle_floor_floor();

void detection_pass() {
  handle_sphere_sphere();
  handle_sphere_rect();
  handle_sphere_floor();
  handle_rect_rect();
  handle_rect_floor();
}
