#pragma once

#include "state.h"

void hud_init();
void hud_update();
void hud_draw();
void hud_clean();

// react to a state change.
void hud_react_to_change(GameState new_state);
