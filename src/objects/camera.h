#ifndef CAMERA_H
#define CAMERA_H

#include "../object_lut.h"

#include "../object.h"
#include "cglm/types.h"
#include "object_bases.h"
#include "physics/physics.h"

typedef struct Camera {
  OBJECT_FIELDS

  PhysComp *phys;

  vec3 *target; // holds a pointer to a position. link another entity's position
                // with this one through a pointer, and trust that the entity
                // will be freed AFTER the camera attached to it is freed. (or
                // else it'll have a garbage target with random data)
  float speed;

  float rotation;       // in radians, around the player.
  float rotation_speed; // how fast does the camera swing around the player?

  float distance; // how far should we stay from the target in the circle?
  float zoom_speed;

  float height; // how far above the player is the camera?
  float rising_speed;
} Camera;

Camera *camera_build(vec3 position, vec3 *target);
void camera_destroy(Camera *c);

void camera_init(void *c);
void camera_update(void *c);
void camera_clean(void *c);

#endif // !CAMERA_H
