#include "hud.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "global.h"
#include "helper_math.h"
#include "main.h"
#include "meshing/font.h"
#include "object.h"
#include "objects/button.h"
#include "objects/label.h"
#include "objects/texture.h"
#include "path.h"
#include "state.h"

Button *b;

Texture *hud_background;

TextureHandle run_texture_idx;
TextureHandle talk_texture_idx;
static Button *run_btn = NULL;
static Button *talk_btn = NULL;

void hud_init() {
  run_texture_idx = g_load_texture(TEXTURE_PATH("run_btn.png"));
  talk_texture_idx = g_load_texture(TEXTURE_PATH("talk_btn.png"));

  // we'll define the boundaries and sizes of a simple monospace font, then just
  // use the currently bound texture for font drawing, and trust that it's the
  // proper font texture that maps to this handle.

  // stride - the number of characters per row (or column) in the font.
  // 16 x 9 font characters means each letter fits in a 16x16 px square, or on a
  // 512 double-size texture, a 32x32px square.
  ui_font =
      FontInit(256, 256, 16, 9,
               0xFFFFFFFF); // make sure the tex size ends up as a power of two.
  // 16 * 9 = 96 cells, which is the num of printable ascii characters.

  glm_mat4_identity(m_ui_model);
  glm_mat4_identity(m_ui_projection);

  float aspect = WIN_W / WIN_H; // Aspect ratio.
  float fov = glm_rad(185.0f);  // Field of view in radians (45 degrees here).
  float nearPlane = 0.1f;       // Near clipping plane.
  float farPlane = 500.0f;      // Far clipping plane.

  // 0 - 1, 0 - 1 orthographic projection to screenspace for the UI. have the UI
  // coordinates not scale with the screensize, so that we can resize the
  // window?
  glm_ortho(0, 1, 0, 1, -5.0f, 5.0f, m_ui_projection);

  hud_background = (Texture *)object_add((Object *)texture_build(
      (AABB){0.0F, 0.0F, 1.0F, 1.0F},
      textures[g_load_texture(TEXTURE_PATH("hud_overlay.png"))]));
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

static void attack_callback() { printf("attacking\n"); }

static void run_callback() {
  printf("running\n");
  state_change(GS_WALKING); // leave the encounter.
}

static void talk_callback() { printf("Hello\n"); }

void hud_react_to_change(GameState new_state) {
  switch (new_state) {
  case GS_MAIN_MENU:
    break;
  case GS_WALKING:
    SAFE_FREE_OBJECT(run_btn);
    SAFE_FREE_OBJECT(talk_btn);
    break;
  case GS_ENCOUNTER:
    printf("hud reaction\n");
    talk_btn = (Button *)object_add(
        (Object *)button_build((AABB){0.2F, 0.0F, 0.1F, 0.1F}, "talk",
                               talk_callback, textures[talk_texture_idx]));
    run_btn = (Button *)object_add(
        (Object *)button_build((AABB){0.7F, 0.0F, 0.1F, 0.1F}, "run",
                               run_callback, textures[run_texture_idx]));
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

void hud_draw() { // then, draw the UI.

  g_use_pipeline(PC_HUD);

  // we don't bind the global hud vao, the objects have their own vaos.
  object_draw_hud(); // call all the hud drawing lifecycle methods on each
                     // object in the world.
}

void hud_clean() {}
