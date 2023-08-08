#include "backends/graphics_api.h"
#include <stddef.h>

void g_use_pipeline(PipelineConfiguration config) {
  // Backend-specific implementation here
}

void g_init() {
  // Backend-specific implementation here
}

GraphicsRender *g_new_render(const float *vertices, unsigned int v_count,
                             const unsigned int *indices, unsigned int i_count,
                             RenderConfiguration conf) {
  // Backend-specific implementation here
  return NULL;
}

void g_draw_render(GraphicsRender *r) {
  // Backend-specific implementation here
}

void g_clean() {
  // Backend-specific implementation here
}
