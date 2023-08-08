#pragma once

#include <stdbool.h>

typedef enum GameState {
  GS_MAIN_MENU, // main menu, select modes and save
  GS_WALKING,   // walking around in a dungeon, between encounters.
  GS_ENCOUNTER, // encounter, the menu is pulled up to select options.
  GS_COUNT,
} GameState;

// just the state enum variant that we're in.
extern GameState game_state;

// bool informs the caller if the state change was new.
bool state_change(GameState new_state);
