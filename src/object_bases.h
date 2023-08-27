#pragma once

#include "cglm/types.h"
#include <stdint.h>

#define PHYS_OBJ_START 0b1000000000
#define PHYS_OBJ_END 0b10000000000

typedef enum ObjectType {
  // hud
  OBJ_LABEL,
  OBJ_BUTTON,

  // we need to be able to quickly discern whether an object is a physics
  // object, use a masking type pattern again.
  OBJ_PLAYER = PHYS_OBJ_START,
  OBJ_CAMERA,

  OBJ_DETECTOR,

  // prims
  OBJ_CUBE,
  OBJ_SPHERE,
  OBJ_FLOOR,

  OBJ_CHARACTER = PHYS_OBJ_END,

  OBJ_COUNT,
} ObjectType;

#define IS_PHYS_OBJECT(o_type)                                                 \
  ((o_type >= PHYS_OBJ_START) && (o_type <= PHYS_OBJ_END))

typedef enum ObjectTag {
  OT_INVALID = 0,

  OT_PERMANENT,
  OT_HUD,
  OT_AREA,

  OT_COUNT,
} ObjectTag;

// and now we can just use it like normal, kind of. instead of renewing a long
// at the most basic level, an object doesn't really have much.
#define OBJECT_FIELDS                                                          \
  ObjectType type;                                                             \
  uint16_t id;                                                                 \
  ObjectTag tag;

// handled in the external LUT, an object is linked to a FnPointers structure by
// its type.
typedef struct Object {
  OBJECT_FIELDS
} Object;
