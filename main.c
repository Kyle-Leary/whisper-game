#include "animation/animator.h"
#include "areas/areas.h"
#include "backends/audio_api.h"
#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "backends/lifecycle_api.h"
#include "cglm/affine-pre.h"
#include "cglm/mat4.h"
#include "cglm/util.h"
#include "core/area_server.h"
#include "core/battle.h"
#include "event_types.h"
#include "global.h"

#include "cglm/cglm.h"
#include "cglm/types.h"

#include "glprim.h"
#include "hud.h"
#include "main.h"
#include "meshing/gltf_mesher.h"
#include "object.h"
#include "path.h"
#include "physics.h"

#include "backends/input_api.h"
#include "meshing/font.h"

#include "helper_math.h"
#include "objects/player.h"

#include "general_lighting.h"
#include "physics/collider_types.h"
#include "printers.h"
#include "size.h"
#include "util.h"
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {
  // call the lifecycle init and give them control before ANYTHING else.
  l_init();

  // delta_time is a global defined in global.h
  clock_t start_time, end_time; // Variables to store the clock cycles

  // glm is general purpose math, this isn't gl-specific. everything uses
  // projection matrices!
  glm_perspective(glm_rad(75.0f), WIN_W / WIN_H, 0.1f, 100.0f, m_projection);
  glm_mat4_identity(m_view);
  glm_mat4_identity(m_model);

  // api init
  i_init();
  g_init();
  a_init();

  // core module init
  battle_init();
  physics_init();
  object_init();
  anim_init();

  area_switch(AREA_LEVEL);

  hud_init();

  start_time = clock();

  /// limit fps to match the main.h macro defined value.
  // Variables for controlling FPS
  clock_t last_time = clock();
  double fps_timer = 0.0;

  // setup textures
  TextureHandle nepeta = textures[g_load_texture(TEXTURE_PATH("nepeta.jpg"))];
  TextureHandle character =
      textures[g_load_texture(TEXTURE_PATH("character.png"))];

  GraphicsRender *skybox_render = glprim_skybox_cube();
  { // setup skybox, bind the texture at the start and on each skybox switch to
    // avoid redundancy.
    char *cubemap_paths[6] = {TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png"),
                              TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png"),
                              TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png")};

    TextureHandle skybox_tex = textures[g_load_cubemap(cubemap_paths)];
    skybox_render->pc = PC_SKYBOX;
    // this also means that you can effectively change the whole skybox without
    // much CPU overhead by simply changing the cubemap texture bound to
    // SKYBOX_TEX_SLOT.
    g_use_cubemap(
        skybox_tex,
        SKYBOX_TEX_SLOT); // the cubemap and texture_2d textures are in the same
                          // texture slot space. binding a texture2d over the
                          // cubemap will overwrite it, so just put the skybox
                          // in it's own special slot.
  }

  bool debug_drawing = false;

  // Loop until the user closes the window
  while (!l_should_close()) {
    clock_t current_time = clock();
    double currentTime = (double)current_time / CLOCKS_PER_SEC;
    fps_timer += (currentTime - last_time);
    last_time = currentTime;

    if (fps_timer < FRAME_TIME) {
      continue;
    }

    fps_timer = 0;

    i_update(); // clear the temporary input state
    l_update(); // potentially poll for events?
    a_update();

    // tick
    physics_update();
    battle_update();
    object_update();
    anim_update();

    {
      if (i_state.act_just_pressed[ACT_TOGGLE_DEBUG_DRAW]) {
        debug_drawing = !debug_drawing;
      }
    }

    hud_update();

    l_begin_draw();

    // after admin matrix stuff is over with, actually draw all the Renders.

    { // draw the 3d scene
      // our top-level, default pipeline.
      { // render skybox
        g_draw_render(skybox_render);
      }

      if (debug_drawing) { // render the debug physics objects.
        physics_debug_draw();
      }

      g_use_texture(nepeta, 0);
      object_draw(); // handle all the individual draw routines for all the
                     // objects in the world.
    }

    l_end_draw();

    // Update start_time for the next iteration
    clock_t end_time = clock();
    double delta_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    start_time = end_time;
  }

  i_clean();
  g_clean();
  a_clean();

  physics_clean();
  object_clean();
  hud_clean();
  anim_clean();

  // LAST, clean up the main function in the backend.
  if (!l_clean()) {
    // error cleaning
    return 1;
  }

  return 0;
}
