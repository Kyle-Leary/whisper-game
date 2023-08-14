#include "detector.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "physics/collider_types.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// really basic Detector entity type.

#define CAST Detector *detector = (Detector *)p

Detector *detector_build(vec3 position, Collider *col) {
  Detector *p = (Detector *)calloc(sizeof(Detector), 1);
  p->type = OBJ_DETECTOR;

  memcpy(p->position, position, sizeof(float) * 3);
  memcpy(p->lerp_position, p->position, sizeof(float) * 3);

  p->num_colliders = 1;
  p->colliders = (Collider *)malloc(sizeof(Collider) * p->num_colliders);
  // proper copy, don't just move the pointer in.
  memcpy(&(p->colliders[0]), col, sizeof(Collider));

  p->position_lerp_speed = 0.9F;
  p->mass = 0.1;
  p->linear_damping = 0.5;

  p->intangible = true;
  p->immovable = true;

  // no explicit render. the physics subsystem should be able to handle its own
  // physics debug renders.
  return p;
}

void detector_init(void *p) {}

void detector_update(void *p) { CAST; }

void detector_handle_collision(void *p, CollisionEvent *e) {}

void detector_clean(void *p) {
  Detector *detector = (Detector *)p;
  free(detector);
}
