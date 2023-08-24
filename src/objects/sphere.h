#ifndef SPHERE_H
#define SPHERE_H

#include "../object_lut.h"

#include "../object.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "physics/component.h"
#include "render.h"

typedef struct Sphere {
  OBJECT_FIELDS

  PhysComp *phys;
  RenderComp *render;
} Sphere;

Sphere *sphere_build(vec3 position, float radius, unsigned int segments);
void sphere_destroy(Sphere *s);

void sphere_init(void *s);
void sphere_update(void *s);
void sphere_clean(void *s);

#endif // !SPHERE_H
