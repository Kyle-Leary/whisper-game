#ifndef AREAOBJ_H
#define AREAOBJ_H

#include "../object_lut.h"

#include "../cglm/types.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "parsers/area.h"

typedef struct AreaObj {
  PHYS_OBJECT_FIELDS

  GraphicsRender *render;
} AreaObj;

AreaObj *areaObj_build(AreaFile *area);
void areaObj_destroy(AreaObj *a);

void areaObj_init(void *a);
void areaObj_update(void *a);
void areaObj_draw(void *a);
void areaObj_clean(void *a);
void areaObj_handle_collision(void *a, CollisionEvent *e);

#endif // !AREAOBJ_H
