#pragma once

#include "cglm/cglm.h"
#include "cglm/types.h"

#include <math.h>

#include <stdbool.h>
#include <string.h>

#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))

#define BETWEEN(float_position, min, max)                                      \
  (float_position > min && float_position < max)

// scuffed compile-time math
#define RADIAN(degree_value) ((M_PI / 180.0F) * degree_value)

// all the stuff that cglm doesn't do.

typedef struct Basis {
  vec3 right;
  vec3 up;
  vec3 forward;
} Basis;

Basis get_basis(mat4 m);

// aabb collision helpers, for stuff like buttons
typedef struct AABB {
  vec2 center;
  vec2 extents;
} AABB;

bool is_point_inside(AABB aabb, vec2 v);

// interpolation/animation helpers.
float lerp(float a, float b, float t);
float exp_interp(float a, float b, float t, float pow);
float quad_interp(float a, float b, float t);
float cubic_interp(float a, float b, float t);
float sin_interp(float a, float b, float t);
float elastic_interp(float a, float b, float t);
float bounce_interp(float a, float b, float t);
float smooth_step(float a, float b, float t);

void lerp_vec3(vec3 a, vec3 b, float t, vec3 dest);

void mat4_lookat_point(mat4 mat, vec3 target);
