#pragma once

// column major.
typedef float vec4[4];
typedef vec4 mat4[4];

void mat4_mul(const mat4 left, const mat4 right, mat4 dest);

// pointwise equality routine.
int mat4_equality(const mat4 a, const mat4 b);

void mat4_identity(mat4 dest);
void mat4_scale(const mat4 from, float scale, mat4 dest);
