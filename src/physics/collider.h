#pragma once

#include "event.h"
#include "event_types.h"
#include "object_bases.h"
#include "physics.h"
#include "physics/collider_types.h"
#include <stdbool.h>

void handle_sphere_collision(PhysComp *base_phys, Collider base_collider,
                             PhysComp *target_phys);

void handle_floor_collision(PhysComp *base_phys, Collider base_collider,
                            PhysComp *target_phys);
