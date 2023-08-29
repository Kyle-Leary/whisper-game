#pragma once

//// DEFINE EVENT TYPES
// objects should push themselves back on a collision.
#include "cglm/types.h"
#include <stdint.h>

// interaction - when you bump/interact with something on the map.
typedef enum InteractionType {
  IT_MOVE_INTO, // just moving into their position.
  IT_EXAMINE,   // actually interacting actively.
  IT_COUNT,
} InteractionType;

// small enough to pass by reference.
typedef struct InteractionEvent {
  InteractionType type;
  int x_pos;
  int y_pos;
} InteractionEvent;

typedef enum InteractionResponseType {
  IRT_NO_HANDLER, // returned when the object isn't even registered to interact
                  // with objects. always keep this set to the 0 variant of this
                  // enum, so we can check this state easily.

  IRT_NONE,      // respond with nothing. passive response.
  IRT_BLOCK,     // block the caller's way.
  IRT_ENCOUNTER, // start up an encounter.
  IRT_COUNT,
} InteractionResponseType;

// each interaction returns a response directly.
typedef struct InteractionResponse {
  InteractionResponseType type;
} InteractionResponse;
