#pragma once

#include "physics/body/body.h"

void m4_apply_transform(mat4 m4, vec3 position, float scale, versor rotation);
void m4_apply_transform_from_body(mat4 m4, Body *b);
