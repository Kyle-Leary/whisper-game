#include "state.h"
#include "hud.h"
#include <stdio.h>
#include <sys/types.h>

// TODO: wtf is this shit

// initial state
GameState game_state = GS_WALKING;

// a blank function that takes in nothing and returns nothing, as a function
// that reacts to a state change.
typedef void (*StateCallback)(
    GameState); // "accessing global variables may cause cache misses more than
                // stack access" => pass the state argument through the stack,
                // rather than through a global?

static const StateCallback callbacks[] = {
    hud_react_to_change,
};

static const uint callback_len = sizeof(callbacks) / sizeof(StateCallback);

bool state_change(GameState new_state) {
  bool is_changed = (new_state != game_state);
  game_state = new_state; // change the state before the callbacks, just in case
                          // the caller uses the global instead.
  if (is_changed) {
    for (int i = 0; i < callback_len; i++) {
#ifdef DEBUG
      if (callbacks[i] == NULL)
        continue;
#endif /* ifdef DEBUG */

      callbacks[i](new_state); // in production, skip the null check. i'm
                               // trusting you ;)
    }
  }
  printf("GameState has been changed from %d to %d.\n", game_state, new_state);
  return is_changed;
}
