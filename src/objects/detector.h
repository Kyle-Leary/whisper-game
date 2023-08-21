#ifndef DETECTOR_H
#define DETECTOR_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"

typedef struct Detector {
  OBJECT_FIELDS

  PhysComp *phys;

  float speed;
  // pass this an event handler to be called.
  CollisionHandler handler;
} Detector;

// pass in a position and a collider pointer that will be copied into the
// colliders on the detector physics object.
Detector *detector_build(vec3 position, Collider *col,
                         CollisionHandler handler);
void detector_destroy(Detector *d);

void detector_init(void *d);
void detector_update(void *d);
void detector_clean(void *d);
void detector_handle_collision(void *d, CollisionEvent *e);

#endif // !DETECTOR_H
