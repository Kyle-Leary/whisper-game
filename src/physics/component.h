#pragma once

#include "physics/body/body.h"
#include "physics/collider/collider.h"

typedef struct PhysComp {
  // these are non-owned references to different values in seperate arrays.
  // it's an intermediate structure, rather than having the collider and body
  // depend on eachother with pointers. (which really isn't necessary)
  Body *body;
  Collider *collider;
} PhysComp;

PhysComp *make_physcomp(Body *body, Collider *collider);
