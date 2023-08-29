#ifndef OBJECT_LUT_H
#define OBJECT_LUT_H

#include <stdint.h>

#include "cglm/cglm.h"
#include "cglm/types.h"
#include "object.h"

// lots of things are going to be reaching into the object_lut, since it also
// contains event handler functions per-object. therefore, it's better to put it
// in it's own compilation medium, to avoid circular inclusion deps.

//// DEFINE FUNCTION TYPES
// everything takes in void pointers, need to be very generic about this.
typedef void (*InitFn)(void *); // when it's added to the objects array.
                                // different from construction.
typedef void (*UpdateFn)(void *);

typedef void (*CleanFn)(void *); // when it's removed from the objects array.
                                 // different from destruction.

// have a LUT of ObjectFnPointers for each type of object.
// the functions themselves are passed a void *, and trusted to cast to the
// appropriate type.
typedef struct ObjectFnPointers {
  InitFn init;
  UpdateFn update;
  CleanFn clean;
} ObjectFnPointers;

extern ObjectFnPointers fn_lut[OBJ_COUNT];

#endif // !OBJECT_LUT_H
