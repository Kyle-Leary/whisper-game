#ifndef OBJECT_H
#define OBJECT_H

#include "cglm/types.h"
#include "event_types.h"
#include "object_bases.h"
#include <stdint.h>

#define NUM_OBJECTS 1000

typedef struct ObjectState {
  Object *objects[NUM_OBJECTS]; // array of pointers.
} ObjectState;

extern ObjectState object_state;

Object *object_add(Object *o, ObjectTag tag);
void object_clear_tag(ObjectTag tag);
// removing by index skips the search entirely and is MUCH FASTER. index in the
// "objects" global array.
void object_remove_by_index(uint16_t index);
void object_remove_by_ptr(Object *o);

void object_init();
void object_update();
void object_draw();
void object_clean();

#endif // !OBJECT_H
