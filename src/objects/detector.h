#ifndef DETECTOR_H
#define DETECTOR_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"

typedef struct Detector {
  PHYS_OBJECT_FIELDS

  float speed;
} Detector;

// pass in a position and a collider pointer that will be copied into the
// colliders on the detector physics object.
Detector *detector_build(vec3 position, Collider *col);
void detector_destroy(Detector *d);

void detector_init(void *d);
void detector_update(void *d);
void detector_clean(void *d);
void detector_handle_collision(void *d, CollisionEvent *e);

#endif // !DETECTOR_H
