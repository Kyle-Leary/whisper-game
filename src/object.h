#ifndef OBJECT_H
#define OBJECT_H

#include "cglm/types.h"
#include "event_types.h"
#include "object_bases.h"
#include <stdint.h>

#define NUM_OBJECTS 200

typedef struct ObjectState {
  Object *objects[NUM_OBJECTS]; // array of pointers.
} ObjectState;

extern ObjectState object_state;

Object *object_add(Object *o);
void object_remove_by_ptr(Object *o);

void object_init();
void object_update();
void object_draw();
void object_interact(InteractionEvent e, InteractionResponse *responses);
void object_draw_hud();
void object_clean();

#endif // !OBJECT_H
