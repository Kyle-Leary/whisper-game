#pragma once

// handle all the collider shape iteration and collision detection in this
// module.

#include "cglm/types.h"
#include "physics/body/body.h"

// generate pairs of these events and put them into eachother's shape event
// queues.
typedef struct CollisionEvent {
  Body *from; // who was this event from?

  vec3 contact_pt;
  vec3 direction; // encode the data of where the object was pushed from,
                  // and how far the push should be.
  float depth;    // how far inside the sender is the contact point?
} CollisionEvent;

void detection_pass();

void handle_sphere_sphere();
void handle_sphere_rect();
void handle_sphere_floor();
void handle_rect_rect();
void handle_rect_floor();
void handle_floor_floor();
