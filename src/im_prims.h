#pragma once

#include "physics/physics.h"

// draw a line from the center of mass representing the velocity of the physics
// object.
void im_velocity(RigidBody *rb);
void im_acceleration(RigidBody *rb);
void im_vector(vec3 origin, vec3 endpoint, vec4 color);
void im_cube(vec3 center, float side_length);
void im_point(vec3 point);
void im_transform(mat4 m, float saturation);
void im_grid(mat4 m, int size, float step);
void im_identity_grid(int size, float saturation);
