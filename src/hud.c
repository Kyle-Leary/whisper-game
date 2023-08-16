#include "hud.h"
#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "cglm/types.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "main.h"
#include "meshing/font.h"
#include "object.h"
#include "object_bases.h"
#include "objects/button.h"
#include "objects/label.h"
#include "objects/texture.h"
#include "path.h"
#include "physics/raycast.h"
#include "state.h"
#include <GL/gl.h>
#include <stdint.h>

Button *b;

Texture *hud_background;

TextureHandle run_texture_idx;
TextureHandle talk_texture_idx;
static Button *run_btn = NULL;
static Button *talk_btn = NULL;

Texture *mouse_cursor;

static void attack_callback() { printf("attacking\n"); }

static void run_callback() {
  printf("running\n");
  state_change(GS_WALKING); // leave the encounter.
}

static void talk_callback() { printf("Hello\n"); }

void hud_init() {
  run_texture_idx = g_load_texture(TEXTURE_PATH("run_btn.png"));
  talk_texture_idx = g_load_texture(TEXTURE_PATH("talk_btn.png"));

  // we'll define the boundaries and sizes of a simple monospace font, then just
  // use the currently bound texture for font drawing, and trust that it's the
  // proper font texture that maps to this handle.

  { // setup the background/border of the HUD.
    hud_background = (Texture *)object_add(
        (Object *)texture_build(
            (AABB){0.0F, 0.0F, 1.0F, 1.0F},
            textures[g_load_texture(TEXTURE_PATH("hud_overlay.png"))]),
        OT_HUD);
  }

  object_add((Object *)label_build(simple_font, (vec2){0.2, 0.15}, "WHISPER",
                                   (vec3){0, 1, 0}),
             OT_HUD);

  { // setup the mouse cursor texture display.
    mouse_cursor = (Texture *)object_add(
        (Object *)texture_build(
            (AABB){0, 0, 0.05f, 0.05f},
            textures[g_load_texture(TEXTURE_PATH("mouse_cursor.png"))]),
        OT_HUD);
  }

  {
    talk_btn = (Button *)object_add(
        (Object *)button_build(simple_font, (AABB){0.2F, 0.0F, 0.1F, 0.1F},
                               "talk", talk_callback,
                               textures[talk_texture_idx]),
        OT_HUD);

    run_btn = (Button *)object_add(
        (Object *)button_build(simple_font, (AABB){0.7F, 0.0F, 0.1F, 0.1F},
                               "run", run_callback, textures[run_texture_idx]),
        OT_HUD);
  }
}

// only remove objects by ptr if they're non-null, the object function shouldn't
// have to run a last-minute check on this. caller imbues the function with
// extra context.
#define SAFE_FREE_OBJECT(ptr)                                                  \
  do {                                                                         \
    if ((ptr) != NULL) {                                                       \
      object_remove_by_ptr((Object *)ptr);                                     \
      (ptr) = NULL; /* Optional: Set the pointer to NULL after freeing */      \
    }                                                                          \
  } while (0)

void hud_react_to_change(GameState new_state) {
  switch (new_state) {
  case GS_MAIN_MENU:
    break;
  case GS_WALKING:
    break;
  case GS_ENCOUNTER:
    // SAFE_FREE_OBJECT(run_btn);
    // SAFE_FREE_OBJECT(talk_btn);
    break;
  default:
    break;
  }
}

void hud_update() {
  // update the mouse cursor texture position to match the actual position, no
  // matter what game mode we're in.
  memcpy(mouse_cursor->aabb.xy, i_state.pointer, sizeof(float) * 2);

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
