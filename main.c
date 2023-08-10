#include "backends/audio_api.h"
#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "backends/lifecycle_api.h"
#include "cglm/mat4.h"
#include "cglm/util.h"
#include "core/area_server.h"
#include "core/battle.h"
#include "global.h"

#include "cglm/cglm.h"
#include "cglm/types.h"

#include "glprim.h"
#include "hud.h"
#include "main.h"
#include "meshing/dungeon.h"
#include "meshing/flat_dungeon.h"
#include "meshing/gltf_mesher.h"
#include "object.h"
#include "objects/camera.h"
#include "objects/character.h"
#include "objects/cube.h"
#include "objects/floor.h"
#include "objects/label.h"
#include "objects/render.h"
#include "objects/sphere.h"
#include "objects/texture.h"
#include "parsers/area.h"
#include "path.h"
#include "physics.h"

#include "backends/input_api.h"
#include "meshing/font.h"

#include "helper_math.h"
#include "objects/player.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

Texture *mouse_cursor;

int main() {
  // call the lifecycle init and give them control before ANYTHING else.
  l_init();

  // delta_time is a global defined in global.h
  clock_t start_time, end_time; // Variables to store the clock cycles

  // glm is general purpose math, this isn't gl-specific. everything uses
  // projection matrices!
  glm_perspective(glm_rad(75.0f), WIN_W / WIN_H, 0.1f, 100.0f, m_projection);
  glm_mat4_identity(m_view_rot);
  glm_mat4_identity(m_view_tf);
  glm_mat4_identity(m_model);

  hud_init();

  // api init
  i_init();
  g_init();
  a_init();

  // core module init
  battle_init();
  physics_init();
  object_init();

  // initialize to a specific area. we can always assume that we're in an
  // "area", no matter what.
  // area_init("larger_test.area");

  Player *player = (Player *)object_add((Object *)player_build());
  player->position[0] = 5;
  player->position[1] = -1; // player will be pushed above the floor
                            // automatically by the physics subsystem.
  object_add((Object *)cube_build((vec3){9, -1, 0}));
  object_add((Object *)cube_build((vec3){0, -1, 9}));
  object_add((Object *)cube_build((vec3){0, -1, -9}));
  object_add((Object *)cube_build((vec3){-9, -1, -9}));
  object_add((Object *)cube_build((vec3){7, -1, -9}));

  object_add((Object *)sphere_build((vec3){2, -1, -2}, 1, 10));

  object_add((Object *)floor_build((vec3){0, -1, 0}, 50));

  // parse then mesh the glb file, then render it in the normal drawing loop.
  GraphicsRender *glb =
      gltf_to_render_simple(gltf_parse(MODEL_PATH("final_boss.glb")));

  glm_translate(glb->model, (vec3){5, -1, 5});

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &player->lerp_position));

  mouse_cursor = (Texture *)object_add((Object *)texture_build(
      (AABB){0, 0, 0.05f, 0.05f},
      textures[g_load_texture(TEXTURE_PATH("mouse_cursor.png"))]));

  // setup textures
  TextureHandle nepeta = textures[g_load_texture(TEXTURE_PATH("nepeta.jpg"))];
  TextureHandle character =
      textures[g_load_texture(TEXTURE_PATH("character.png"))];

  start_time = clock();

  /// limit fps to match the main.h macro defined value.
  // Variables for controlling FPS
  clock_t last_time = clock();
  double fps_timer = 0.0;

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

    // update the mouse cursor texture position to match the actual position.
    memcpy(mouse_cursor->aabb.xy, i_state.pointer, sizeof(float) * 2);

    // tick
    physics_update();
    battle_update();
    object_update();

    hud_update();

    l_begin_draw();

    // after admin matrix stuff is over with, actually draw all the Renders.

    { // draw the 3d scene
      g_use_pipeline(PC_BASIC);
      g_use_texture(nepeta);
      g_draw_render(glb);
      object_draw(); // handle all the individual draw routines for all the
                     // objects in the world.
    }

    hud_draw();

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

  // LAST, clean up the main function in the backend.
  if (!l_clean()) {
    // error cleaning
    return 1;
  }

  return 0;
}
