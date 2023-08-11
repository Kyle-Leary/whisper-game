#include "backends/graphics_api.h"
#include "backends/linux/graphics/shader.h"
#include "backends/linux/ogl_includes.h"
#include "linux_graphics_globals.h"
#include <stdio.h>

Material *curr_mat = NULL;

// use the material, in this case that means setting all the right values on the
// active shader.
void g_use_material(Material *mat) {
  if (mat == NULL) {
    fprintf(stderr, "Cannot g_use a NULL material.\n");
    return;
  }
  curr_mat = mat;

  // Set albedo (assuming vec4 is a float[4])
  shader_set_4f(curr_program, "mat1.albedo", mat->albedo[0], mat->albedo[1],
                mat->albedo[2], mat->albedo[3]);

  // Set metallic and roughness
  shader_set_1f(curr_program, "mat1.metallic", mat->metallic);
  shader_set_1f(curr_program, "mat1.roughness", mat->roughness);

  // samplers
  shader_set_1i(curr_program, "mat1.base_color_texture",
                mat->base_color_texture);
  shader_set_1i(curr_program, "mat1.metallic_roughness_texture",
                mat->metallic_roughness_texture);
  shader_set_1i(curr_program, "mat1.normal_texture", mat->normal_texture);
  shader_set_1i(curr_program, "mat1.occlusion_texture", mat->occlusion_texture);
  shader_set_1i(curr_program, "mat1.emissive_texture", mat->emissive_texture);

  // Set emissive factor
  shader_set_3f(curr_program, "mat1.emissive_factor", mat->emissive_factor[0],
                mat->emissive_factor[1], mat->emissive_factor[2]);

  // Set double-sided
  shader_set_1i(curr_program, "mat1.double_sided", mat->double_sided);
}
