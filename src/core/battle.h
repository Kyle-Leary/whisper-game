#pragma once

#include "defines.h"
#include "state.h"

// maximum number of enemies in any given battle.
#define NUM_ENEMIES 5

// how fast things in battle update.
#define TICKS_PER_SECOND 5
// how long does it take to tick?
#define TICK_TIME (TICKS_PER_SECOND / 60)

typedef struct Stats {
  u32 defense;
  u32 attack;
  float recover_rate; // between 0 and 1, used as a lerp percentage.
} Stats;

typedef struct CharacterBattleState {
  Stats stats; // parameters, used in calculation

  // current values, modified by calculations
  float loss_rate; // how fast are we currently losing health? maybe have a
                   // battle "tick" system that's different from just doing this
                   // each frame?
  float health;    // 0 to 100.0F? does there need to be max health?
} CharacterBattleState;

// posed like a module.
// the battle module holds its own state, and is purely reflected in the object
// system by other modules. ideally, this shouldn't render anything on its own?
typedef struct BattleState {
  CharacterBattleState *player;
  CharacterBattleState *enemy; // only one enemy!!
} BattleState;

extern BattleState battle_state;

void battle_init();

// handle the ticking mechanisms.
void battle_update();

void battle_add_enemy(CharacterBattleState *e);
void battle_attack_enemy(float power);

void battle_handle_state_change(GameState new_state);
