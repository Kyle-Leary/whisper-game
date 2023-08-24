#ifndef CHARACTER_H
#define CHARACTER_H

#include "../object_lut.h"

#include "../object.h"
#include "animation/animator.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "core/area_server.h"
#include "physics/component.h"
#include "render.h"

typedef struct Character {
  OBJECT_FIELDS

  PhysComp *phys;
  RenderComp *render;
  Animator *animator;

  float speed;
} Character;

Character *character_build(Model *model);
void character_destroy(Character *c);

void character_init(void *c);
void character_update(void *c);
void character_clean(void *c);

#endif // !CHARACTER_H
