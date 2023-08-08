#include "object.h"

#include "event_types.h"
#include "object_lut.h"
#include "objects/player.h"
#include "physics.h"

#include <signal.h>
#include <stddef.h>

ObjectState object_state = {
    {0}}; // zero alloc the object pointer array to start off with.

// TODO: is there some way to know if we've forgotten to set handlers for a
// specific object type? use the debug mode to do everything at compile-time.
//
// return the pointer as a formality, since the caller will need
// that to use remove_by_ptr on the objects array through this interface.
Object *object_add(Object *o) {
  if (o == NULL) {
    printf("Cannot add null object to the object table.\n");
    return NULL;
  }

  for (int i = 0; i < NUM_OBJECTS; i++) {
    if (object_state.objects[i] == NULL) {
      // insert into the first NULL slot of the objects array.
      object_state.objects[i] = o;
      fn_lut[o->type].init((void *)o);

#ifdef DEBUG
      // then, launch some debug checks on objects for common segfault spots.
      if (IS_PHYS_OBJECT(o->type)) {
        PhysicsObject *po = (PhysicsObject *)o;
        if (po->num_colliders > 10) {
          printf("WARNING: num_colliders on one object is very high, possibly "
                 "uninitted?\n");
        }
      }
#endif /* ifdef DEBUG */

      return o;
    }
  }

  fprintf(stderr, "Ran out of Object slots in the objects array!");
  // TODO: make proper crash screen
  return NULL;
}

// simple remove by pointer, with dumb search algorithm. potentially find a
// better solution for searching and erasing from the list? this seems
// important.
void object_remove_by_ptr(Object *o) {
  for (int i = 0; i < NUM_OBJECTS; i++) {
    if (object_state.objects[i] == o) {
      fn_lut[o->type].clean((void *)o);
      object_state.objects[i] = NULL; // nullify the pointer, so that the search
                                      // algorithms know the slot is free now.
      return;
    }
  }

  fprintf(stderr,
          "Could not remove object at location %p of type %d, object not found "
          "in the array.",
          o, o->type);
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

void object_draw() {
  for (int i = 0; i < NUM_OBJECTS; i++) {
    Object *o = object_state.objects[i];
    if (o != NULL) {
      fn_lut[o->type].draw((void *)o);
    }
  }
}

void object_draw_hud() {
  for (int i = 0; i < NUM_OBJECTS; i++) {
    Object *o = object_state.objects[i];
    if (o != NULL) {
      fn_lut[o->type].draw_hud((void *)o);
    }
  }
}

// allocate and pass in a responses array of at least size NUM_OBJECTS.
void object_interact(InteractionEvent e, InteractionResponse *responses) {
  for (int i = 0; i < NUM_OBJECTS; i++) {
    Object *o = object_state.objects[i];
    if (o != NULL) {
      // just use the object id as the index. that's gotta be helpful somehow.
      responses[i] = fn_lut[o->type].interact_handler((void *)o, e);
    }
  }
}

// again, this doesn't call the function, it cleans up the overall ObjectState
// subsystem.
void object_clean() {}
