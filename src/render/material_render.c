#include "material_render.h"

#include "macros.h"
#include "render/graphics_render.h"
#include "render/material.h"

void g_init_mat_render(MaterialRender *mr, GraphicsRender *graphics_render) {
  mr->gr = graphics_render;
}

void g_draw_mat_render(MaterialRender *mr) {
  // basically just a wrapper around g_draw_render and GraphicsRender in
  // general.
  g_use_material(&mr->mat);
  g_draw_render(mr->gr);
}

void free_mat_render(MaterialRender *mr) { free_graphics_render(mr->gr); }
