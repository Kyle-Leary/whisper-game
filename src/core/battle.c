#include "battle.h"
#include "global.h"
#include "helper_math.h"
#include "state.h"
#include <string.h>

// zero-init the enemies array and everything else, just say fuckit and handle
// everything in an initializer function.
BattleState battle_state = {0};

void battle_init() {
  battle_state.player =
      (CharacterBattleState *)malloc(sizeof(CharacterBattleState));
  memcpy(battle_state.player,
         &(CharacterBattleState){{10, 10, 0.1F}, 0.01F, 100.0F},
         sizeof(CharacterBattleState));
  // the enemies array is already NULLed out, we don't really need to worry
  // about that.
}

static void update_character(CharacterBattleState *c) {
  c->health -= c->loss_rate;
  c->loss_rate = lerp(c->loss_rate, 0, c->stats.recover_rate);
}

static float tick_timer;

// stuff like the health can still update outside of battle? should a different
// subsystem handle the stats and modification of player health?
void battle_update() {
  tick_timer += delta_time;
  if (tick_timer < TICK_TIME) {
    return;
  }

  tick_timer = 0;
  // we're on a new tick, update everything.

  { // handle generic character updates.
    if (battle_state.player)
      update_character(battle_state.player);
    if (battle_state.enemy)
      update_character(battle_state.enemy);
  }

  // printf("player health: %.2f\n", battle_state.player->health);
}

void battle_attack_enemy(float power) {
  if (!battle_state.enemy) {
    printf("Cannot attack a NULL enemy.\n");
  }

  float rate_diff = battle_state.player->stats.attack * power;

  battle_state.enemy->loss_rate -= rate_diff;

  printf("rate_diff %f done to the enemy!\n", rate_diff);
}

void battle_set_enemy(CharacterBattleState *e) {
  battle_state.enemy = e;
  printf("Set battling enemy to %p.\n", e);
}

// pass in a pointer, we'll fill it here.
static void make_battlestate(CharacterBattleState *e) {
  e->loss_rate = 0;
  e->health = 100.0F;
  e->stats.attack = 10;
  e->stats.defense = 10;
  e->stats.recover_rate = 0.1F;
}

// try to automatically insert the right data into the enemy data based on the
// state change and some global encounter information.
void battle_handle_state_change(GameState new_state) {
  switch (new_state) {
  case GS_ENCOUNTER: {
    CharacterBattleState *s = malloc(sizeof(CharacterBattleState));
    make_battlestate(s);
    battle_set_enemy(s);
  } break;
  case GS_WALKING: {
    battle_set_enemy(NULL);
  } break;
  default:
    break;
  }
}
