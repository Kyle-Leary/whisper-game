#ifndef CHARACTER_H
#define CHARACTER_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "core/area_server.h"

typedef struct Character {
  PHYS_OBJECT_FIELDS

  float speed;
  Model *model;
} Character;

Character *character_build(Model *model);
void character_destroy(Character *c);

void character_init(void *c);
void character_update(void *c);
void character_draw(void *c);
void character_clean(void *c);
void character_handle_collision(void *c, CollisionEvent *e);
InteractionResponse character_handle_interact(void *p, InteractionEvent e);

#endif // !CHARACTER_H
