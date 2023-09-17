#include "floor.h"

#include "../object.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"

#include "helper_math.h"
#include "input_help.h"
#include "mathdef.h"
#include "physics/body/body.h"
#include "physics/collider/collider.h"
#include "printers.h"

#include "render/gr_prim.h"
#include "render/graphics_render.h"
#include "render/material_render.h"
#include "render/render.h"
#include "render/texture.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// mrs are not externally managed like grs? this is very confusing.
static MaterialRender mr = {.mat = {.base_color_texture = 0}};

#define CAST Floor *floor = (Floor *)p

Floor *floor_build(vec3 position) {
  Floor *p = (Floor *)calloc(sizeof(Floor), 1);
  p->type = OBJ_FLOOR;

  {
    p->phys = make_physcomp(
        (Body *)make_static_body(0.5, position, 1.0, IDENTITY_VERSOR),
        (Collider *)make_floor_collider());
  }

  StaticBody *sb = (StaticBody *)p->phys->body;

  GraphicsRender *gr = gr_prim_floor_plane(sb->position);
  gr->shader = get_shader("fraglight");
  glm_scale(gr->model, (vec3){100, 1, 100});
  glm_translate(gr->model, position);

  mr.mat.base_color_texture = nepeta_tex;
  g_init_mat_render(&mr, gr);

  p->render = make_rendercomp_from_matrender(&mr);

  GraphicsRender *prim = (GraphicsRender *)p->render->data.mr->gr;
  return p;
}

void floor_init(void *p) {}

void floor_update(void *p) {}

void floor_clean(void *p) {
  Floor *floor = (Floor *)p;
  free(floor);
}
