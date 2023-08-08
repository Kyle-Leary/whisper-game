#ifndef CAMERA_H
#define CAMERA_H

#include "../object_lut.h"

#include "../cglm/types.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"

typedef struct Camera {
  PHYS_OBJECT_FIELDS

  vec3 *target; // holds a pointer to a position. link another entity's position
                // with this one through a pointer, and trust that the entity
                // will be freed AFTER the camera attached to it is freed. (or
                // else it'll have a garbage target with random data)
  float speed;
} Camera;

Camera *camera_build(vec3 position, vec3 *target);
void camera_destroy(Camera *c);

void camera_init(void *c);
void camera_update(void *c);
void camera_clean(void *c);
void camera_handle_collision(void *c, CollisionEvent *e);

#endif // !CAMERA_H
