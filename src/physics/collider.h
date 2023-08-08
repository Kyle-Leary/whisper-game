#pragma once
#include "event_types.h"
#include "object_bases.h"
#include "physics/collider_types.h"

void handle_sphere_collision(PhysicsObject *base_obj, Collider base_collider,
                             PhysicsObject *target_obj, CollisionEvent *e);
