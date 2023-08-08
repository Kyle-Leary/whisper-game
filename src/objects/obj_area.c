#include "obj_area.h"

#include "../cglm/types.h"
#include "../cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "glprim.h"
#include "input_help.h"
#include "meshing/dungeon.h"
#include "parsers/area.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic AreaObj entity type.

#define CAST AreaObj *areaObj = (AreaObj *)p

AreaObj *areaObj_build(AreaFile *area) {
  AreaObj *p = (AreaObj *)malloc(sizeof(AreaObj));
  p->num_colliders = 0;
  p->type = OBJ_AREAOBJ;
  p->render = make_dungeon_render(area, (vec3){0, 0, 0});
  return p;
}

void areaObj_init(void *p) {}

void areaObj_update(void *p) {}

void areaObj_draw(void *p) {
  CAST;
  g_draw_render(areaObj->render);
}

void areaObj_clean(void *p) {
  AreaObj *areaObj = (AreaObj *)p;
  free(areaObj);
}
