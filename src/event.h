#pragma once

#include "cglm/types.h"
#include "physics/collider_types.h"

typedef struct PhysicsEvent {
  ColliderType sender_col_type;
  vec3 normal;
  float magnitude;
  vec3 contact_pt;
} PhysicsEvent;
