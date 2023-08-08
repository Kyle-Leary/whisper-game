#include "backends/input_api.h"
#include <string.h>

InputState i_state;

void i_init() {}

void i_update() {
  // Update input state here...

  // Reset just pressed and just released actions
  memset(i_state.act_just_pressed, 0, sizeof(i_state.act_just_pressed));
  memset(i_state.act_just_released, 0, sizeof(i_state.act_just_released));
}

void i_clean() {
  // Clean up input state here...
}
