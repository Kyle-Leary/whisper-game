#ifndef INPUT_H
#define INPUT_H

// i_ is the namespace for the input api.

#include "cglm/cglm.h"
#include "cglm/types.h"
#include "defines.h"
#include <stdbool.h>

// there are ActionTypes, which are on-and-off states stored in a simple array.
// then, for stuff like the joystick, there are special inputs in the InputState
// structure for those.
typedef enum ActionType {
  ACT_ZERO, // lol, because of the way this is implemented, the 0th act is
            // dangerous to use. so just assign it to a fake action and dump it.
  ACT_UP,
  ACT_DOWN,
  ACT_LEFT,
  ACT_RIGHT,

  ACT_WORLD_INTERACT,

  ACT_IS_SWINGING, // eg, the mouse is held down to init a swing on computer.

  ACT_STRAFE_LEFT,
  ACT_STRAFE_RIGHT,

  ACT_HUD_INTERACT, // hud interaction action, like clicking on a button.

  ACT_COUNT, // NO, COUNT IS NOT AN ACTION!!!
} ActionType;

// everything shares a common input state, but has different update files that
// setup and flip the input state bits, depending on the input handlers.
typedef struct InputState {
  /// define etc inputs, like sticks and weird datapoints
  float stick_x;
  float stick_y;

  // either the mouse or an arbitrary "pointer" object
  // for the screen.
  vec2 pointer;
  vec2 pointer_velocity; // helper
  vec2 prev_pointer;     // we're operating on this in batch, all the pointer
                     // characteristics in a row. thus, it makes sense to store
                     // them all contiguously.

  /// then, define button-like binary inputs
  // for example, if (act_held[ACT_UP]) { // ACT_UP is being held }
  u8 act_held[ACT_COUNT];
  u8 act_just_pressed[ACT_COUNT];
  u8 act_just_released[ACT_COUNT];

  u8 last_act_state[ACT_COUNT]; // We need to remember the last state for each
                                // button
} InputState;

extern InputState i_state;

void i_init();
void i_update();
void i_clean();

#endif
