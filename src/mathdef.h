#pragma once

// defines for math/geometry literals

#include <cglm/cglm.h>

// Vector Constants
#define ZERO_VEC3 ((vec3){0, 0, 0})
#define ZERO_VEC4 ((vec4){0, 0, 0, 0})
#define UNIT_X_VEC3 ((vec3){1, 0, 0})
#define UNIT_Y_VEC3 ((vec3){0, 1, 0})
#define UNIT_Z_VEC3 ((vec3){0, 0, 1})
#define UNIT_X_VEC4 ((vec4){1, 0, 0, 0})
#define UNIT_Y_VEC4 ((vec4){0, 1, 0, 0})
#define UNIT_Z_VEC4 ((vec4){0, 0, 1, 0})
#define UNIT_W_VEC4 ((vec4){0, 0, 0, 1})

// Versor Constants
#define IDENTITY_VERSOR ((versor){0, 0, 0, 1})

// Matrix Constants
#define IDENTITY_MAT3 ((mat3){{1, 0, 0}, {0, 1, 0}, {0, 0, 1}})
#define IDENTITY_MAT4                                                          \
  ((mat4){{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}})

// Angle Constants
#define PI 3.14159265358979323846f
#define HALF_PI (PI / 2.0f)
#define TWO_PI (2.0f * PI)

// Common Scaling Constants
#define SCALE_HALF_VEC3 ((vec3){0.5f, 0.5f, 0.5f})
#define SCALE_DOUBLE_VEC3 ((vec3){2.0f, 2.0f, 2.0f})

// Common Color Constants
#define WHITE_COLOR_VEC3 ((vec3){1.0f, 1.0f, 1.0f})
#define BLACK_COLOR_VEC3 ((vec3){0.0f, 0.0f, 0.0f})
#define RED_COLOR_VEC3 ((vec3){1.0f, 0.0f, 0.0f})
#define GREEN_COLOR_VEC3 ((vec3){0.0f, 1.0f, 0.0f})
#define BLUE_COLOR_VEC3 ((vec3){0.0f, 0.0f, 1.0f})
