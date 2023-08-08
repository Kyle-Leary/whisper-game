#pragma once

#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "glprim.h"
#include "helper_math.h"

GraphicsRender *glprim_cube(vec3 position);
GraphicsRender *glprim_upright_plane(vec3 position);
GraphicsRender *glprim_floor_plane(vec3 position);
GraphicsRender *glprim_stack_cube(vec3 position);
GraphicsRender *glprim_ui_rect(AABB aabb);
GraphicsRender *glprim_sphere(vec3 position, float radius,
                              unsigned int segments);
