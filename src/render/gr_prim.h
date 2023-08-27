#pragma once

#include "cglm/types.h"
#include "helper_math.h"
#include "render/graphics_render.h"

GraphicsRender *gr_prim_cube(vec3 position);
GraphicsRender *gr_prim_rect(vec3 extents);

GraphicsRender *gr_prim_skybox_cube();
GraphicsRender *gr_prim_upright_plane(vec3 position);
GraphicsRender *gr_prim_floor_plane(vec3 position);
GraphicsRender *gr_prim_stack_cube(vec3 position);

// either center it about the origin, or have the width and height sticking out
// from zero.
GraphicsRender *gr_prim_ui_rect(AABB aabb, bool centered);

GraphicsRender *gr_prim_sphere(vec3 position, float radius,
                               unsigned int segments);
