#include "render.h"

#include "../cglm/types.h"
#include "../cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "glprim.h"

#include "input_help.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic render entity type.

#define CAST RenderObject *render = (RenderObject *)p

// pass in the render manually.
RenderObject *render_build(GraphicsRender *render) {
  RenderObject *p = (RenderObject *)malloc(sizeof(RenderObject));
  p->type = OBJ_RENDER;
  p->render = render;
  printf("making a render object with render %p\n", p->render);
  return p;
}

void render_destroy(RenderObject *c) {}

void render_draw(void *p) {
  CAST;
  g_draw_render(render->render);
}

void render_clean(void *p) {
  RenderObject *render = (RenderObject *)p;
  free(render);
}
