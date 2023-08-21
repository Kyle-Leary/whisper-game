#include "detector.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "event_types.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "object_lut.h"
#include "physics/collider_types.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAST Detector *detector = (Detector *)p

Detector *detector_build(vec3 position, Collider *col,
                         CollisionHandler handler) {
  Detector *p = (Detector *)calloc(sizeof(Detector), 1);
  p->type = OBJ_DETECTOR;

  {
    Collider *colliders = (Collider *)calloc(sizeof(Collider), 1);
    memcpy(&(colliders[0]), col, sizeof(Collider));

    p->phys =
        make_physcomp(0.1, 1.0, 0.5, true, true, colliders, 1, position, true);
  }

  p->handler = handler;

  // no explicit render. the physics subsystem can handle its own
  // physics debug renders.
  return p;
}

void detector_init(void *p) {}

void detector_update(void *p) { CAST; }

void detector_handle_collision(void *p, CollisionEvent *e) {
  CAST;
  detector->handler(p, e);
}

void detector_clean(void *p) {
  Detector *detector = (Detector *)p;
  free(detector);
}
