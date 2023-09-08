#include "animation/animator.h"
#include "areas/area_server.h"
#include "audio/audio.h"
#include "cglm/cglm.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "console/commands.h"
#include "console/console.h"
#include "event_types.h"
#include "global.h"
#include "gui/gui.h"
#include "gui/widgets.h"
#include "helper_math.h"
#include "hud.h"
#include "immediate.h"
#include "input/input.h"
#include "libav/fmv.h"
#include "main.h"
#include "meshing/font.h"
#include "object.h"
#include "os.h"
#include "path.h"
#include "physics/physics.h"
#include "printers.h"
#include "render/gr_prim.h"
#include "render/render.h"
#include "render/texture.h"
#include "shaders/shader_instances.h"
#include "shaders/ubo.h"
#include "size.h"
#include "timescale.h"
#include "util.h"
#include "window.h"

#include "hot_reload/hot_reload.h"

#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "entry.h"

Font *simple_font = NULL;

int entry_point(int argc, char **argv) {
  srand(time(NULL));

  os_init();

  // expose platform-specific time variables into the entry_point namespace.
  TIME_INIT();

  window_init();

  init_helper_textures();

  hot_reload_init();

  // glm is general purpose math, this isn't gl-specific. everything uses
  // projection matrices!
  glm_perspective(glm_rad(75.0f), (float)win_w / win_h, 0.1f, 100.0f,
                  m_projection);
  glm_mat4_identity(m_view);
  glm_mat4_identity(m_model);

  { // setup fonts
    // stride - the number of characters per row (or column) in the font.
    // 16 x 9 font characters means each letter fits in a 16x16 px square, or on
    // a 512 double-size texture, a 32x32px square.
    simple_font = font_init(16, 16,
                            g_load_texture(TEXTURE_PATH(
                                "ui_font.png"))); // make sure the tex size
                                                  // ends up as a power of two.
    // 16 * 9 = 96 cells, which is the num of printable ascii characters.
  }

  ubo_init();

  // call shader init before anything that uses a global shader, like the gui or
  // console.
  shader_instantiate_all();

  gui_init();

  init_commands();
  console_init();

  // api init
  i_init();
  a_init();

  // core module init
  physics_init();
  object_init();
  anim_init();
  render_init();
  im_init();

  area_init();
  area_switch("video.c");

  hud_init();

  timescale_init();

  // delta_time is a global defined in global.h
  whisper_time_t start_time, end_time;
  GET_TIME(start_time);

  /// limit fps to match the main.h macro defined value.
  // Variables for controlling FPS
  clock_t last_time = clock();
  double fps_timer = 0.0;

  // setup textures
  TextureHandle nepeta = g_load_texture(TEXTURE_PATH("nepeta.jpg"));
  TextureHandle character = g_load_texture(TEXTURE_PATH("character.png"));

  GraphicsRender *skybox_render = gr_prim_skybox_cube();
  char *cubemap_paths[6] = {TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png"),
                            TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png"),
                            TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png")};

  TextureHandle skybox_tex = g_load_cubemap(cubemap_paths);
  skybox_render->shader = get_shader("skybox");

  bool debug_drawing = false;

  // Loop until the user closes the window
  while (!window_should_close()) {
    fps_timer += delta_time;
    if (fps_timer < frame_time) {
      continue;
    }

    fps_timer = 0;

    u_time += delta_time;

    ubo_update();

    console_update();
    gui_update();

    area_update();

    window_update(); // potentially poll for events?
    a_update();

    {
      if (i_state.act_just_pressed[ACT_TOGGLE_DEBUG_DRAW]) {
        debug_drawing = !debug_drawing;
      } else if (i_state.act_just_pressed[ACT_TOGGLE_DEBUG_CONSOLE]) {
        toggle_console();
      }
    }

    // tick
    physics_update();
    object_update();
    anim_update();
    render_update();
    timescale_update();

    hud_update();

    // clear the window, anything drawn in an update() function before this will
    // not show.
    window_begin_draw();

    // after admin matrix stuff is over with, actually draw all the Renders.

    { // draw the 3d scene (YES, ORDER MATTERS HERE)
      // our top-level, default pipeline.
      { // render skybox
        // g_use_cubemap(skybox_tex, SKYBOX_TEX_SLOT);
        // g_draw_render(skybox_render);
      }
      g_use_texture(nepeta, 0);
      render_draw();

      // draw all the immediate mode stuff with the im 3d positional shader.
      im_flush();

      if (debug_drawing) { // render the debug physics objects.
        physics_debug_draw();
      }

      gui_draw();
      console_draw();
    }

    window_end_draw();

    i_update(); // clear the temporary input state

    // Update start_time for the next iteration
    GET_TIME(end_time);
    double delta_time = TIME_DIFF_SECONDS(start_time, end_time);
    start_time = end_time;
  }

  clean_all();

  return 0;
}

void clean_all() {
  console_clean();
  clean_commands();
  gui_clean();

  shader_destroy_all();

  i_clean();
  a_clean();

  physics_clean();
  object_clean();
  hud_clean();
  anim_clean();

  area_clean();

  // LAST, clean up the main function in the backend.
  window_clean();

  hot_reload_clean();

  printf("Everything cleaned successfully.\n");
}
