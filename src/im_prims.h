#pragma once

#include "physics/physics.h"

// draw a line from the center of mass representing the velocity of the physics
// object.
void im_velocity(RigidBody *rb);
void im_acceleration(RigidBody *rb);
void im_point(vec3 point);
