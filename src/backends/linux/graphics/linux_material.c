#include "backends/graphics_api.h"
#include "backends/linux/ogl_includes.h"

Material *curr_mat = NULL;

void g_use_material(Material *mat) { curr_mat = mat; }
