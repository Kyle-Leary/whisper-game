#pragma once

#include "cglm/types.h"
#include "render/render_configuration.h"
#include "shaders/shader_instances.h"

#include "graphics_render.h"
#include "material.h"

typedef struct MaterialRender {
  GraphicsRender *gr;
  Material mat;
} MaterialRender;

void g_init_mat_render(MaterialRender *mr, GraphicsRender *graphics_render);
void g_draw_mat_render(MaterialRender *mr);
