#include "hud.h"
#include "cglm/types.h"
#include "global.h"
#include "helper_math.h"
#include "main.h"
#include "meshing/font.h"
#include "object.h"
#include "object_bases.h"
#include "path.h"
#include "state.h"

#include "physics/raycast_detection.h"

#include <stdint.h>

TextureHandle run_texture_idx;
TextureHandle talk_texture_idx;

static void attack_callback() { printf("attacking\n"); }

static void run_callback() {
  printf("running\n");
  state_change(GS_WALKING); // leave the encounter.
}

static void talk_callback() { printf("Hello\n"); }

void hud_init() {
  run_texture_idx = g_load_texture(TEXTURE_PATH("run_btn.png"));
  talk_texture_idx = g_load_texture(TEXTURE_PATH("talk_btn.png"));
}

void hud_react_to_change(GameState new_state) {
  switch (new_state) {
  case GS_MAIN_MENU:
    break;
  case GS_WALKING:
    break;
  case GS_ENCOUNTER:
    break;
  default:
    break;
  }
}

void hud_update() {
  switch (game_state) {
  case GS_WALKING:
    break;
  case GS_ENCOUNTER:
    break;
  case GS_MAIN_MENU:
    break;
  default:
    break;
  }
}

void hud_clean() {}
