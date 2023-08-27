#include "object.h"

#include "event_types.h"
#include "object_bases.h"
#include "object_lut.h"
#include "objects/player.h"
#include "physics/physics.h"

#include <signal.h>
#include <stddef.h>
#include <stdint.h>

ObjectState object_state = {
    {0}}; // zero alloc the object pointer array to start off with.

// TODO: is there some way to know if we've forgotten to set handlers for a
// specific object type? use the debug mode to do everything at compile-time.
//
// return the pointer as a formality, since the caller will need
// that to use remove_by_ptr on the objects array through this interface.
Object *object_add(Object *o, ObjectTag tag) {
  if (o == NULL) {
    printf("Cannot add null object to the object table.\n");
    return NULL;
  }

  o->tag = tag;

  for (int i = 0; i < NUM_OBJECTS; i++) {
    if (object_state.objects[i] == NULL) {
      // insert into the first NULL slot of the objects array.
      object_state.objects[i] = o;
      o->id = i;
      fn_lut[o->type].init((void *)o);

      return o;
    }
  }

  fprintf(stderr, "Ran out of Object slots in the objects array!\n");
  // TODO: make proper crash screen
  return NULL;
}

void object_remove_by_index(uint16_t index) {
  Object *o = object_state.objects[index];
  fn_lut[o->type].clean((void *)o);
  object_state.objects[index] = NULL;
}

void object_remove_by_ptr(Object *o) {
  for (int i = 0; i < NUM_OBJECTS; i++) {
    if (object_state.objects[i] == o) {
      object_remove_by_index(i);
      return;
    }
  }

  fprintf(stderr,
          "Could not remove object at location %p of type %d, object not found "
          "in the array.\n",
          o, o->type);
}

void object_clear_tag(ObjectTag tag) {
  for (int i = 0; i < NUM_OBJECTS; i++) {
    Object *o = object_state.objects[i];
    if (o && o->tag == tag) {
      printf("removing\n");
      object_remove_by_index(i);
    }
  }
}

// just init the objectstate. calling the init lifecycle method should happen in
// the object_add function.
void object_init() {}

// main lifecycle loops that call every other sub-lifecycle function in each
// object.
void object_update() {
  for (int i = 0; i < NUM_OBJECTS; i++) {
    Object *o = object_state.objects[i];
    if (o != NULL) {
      fn_lut[o->type].update((void *)o);
    }
  }
}

// again, this doesn't call the function, it cleans up the overall ObjectState
// subsystem.
void object_clean() {}
