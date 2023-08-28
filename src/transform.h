#pragma once

#include "helper_math.h"
#include "physics/body/body.h"

// composability of gui transforms is something we need for window nesting in
// the gui layer.
void aabb_apply_transform(AABB *to, AABB *by, AABB *dest);

void m4_apply_transform(mat4 m4, vec3 position, float scale, versor rotation);
void m4_apply_transform_from_body(mat4 m4, Body *b);
