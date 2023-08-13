#include "animation/animator.h"
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

#include "general_lighting.h"
#include "size.h"
#include "util.h"
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
  glm_mat4_identity(m_view);
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
  anim_init();

  // initialize to a specific area. we can always assume that we're in an
  // "area", no matter what.
  // area_init("larger_test.area");

  Player *player = (Player *)object_add((Object *)player_build());
  player->position[0] = 5;
  player->position[1] = -1;

  Character *chr = (Character *)object_add((Object *)character_build(
      gltf_to_model(gltf_parse(MODEL_PATH("suzanne.glb")))));
  chr->position[0] = -4;
  chr->position[1] = -1;

  object_add((Object *)cube_build((vec3){9, -1, 0}));
  object_add((Object *)cube_build((vec3){0, -1, 9}));
  object_add((Object *)cube_build((vec3){0, -1, -9}));
  object_add((Object *)cube_build((vec3){-9, -1, -9}));
  object_add((Object *)cube_build((vec3){7, -1, -9}));

  // object_add((Object *)sphere_build((vec3){2, -1, -2}, 1, 10));

  object_add((Object *)floor_build((vec3){0, -1, 0}, 50));

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &player->lerp_position));

  mouse_cursor = (Texture *)object_add((Object *)texture_build(
      (AABB){0, 0, 0.05f, 0.05f},
      textures[g_load_texture(TEXTURE_PATH("mouse_cursor.png"))]));

  start_time = clock();

  {   // setup light data with some defualts.
    { // setup ambient light
      g_light_data.ambient_light.color[0] = 1.0f;
      g_light_data.ambient_light.color[1] = 0.5f;
      g_light_data.ambient_light.color[2] = 0.5f;
      g_light_data.ambient_light.color[3] = 1.0f;
      g_light_data.ambient_light.intensity = 0.5f;
    }

    {
      PointLight pl;

      pl.intensity = 1.0f;
      pl.position[0] = 0.0f;
      pl.position[1] = 0.0f;
      pl.position[2] = 0.0f;
      pl.color[0] = 0.5f;
      pl.color[1] = 0.1f;
      pl.color[2] = 0.5f;
      pl.color[3] = 1.0f;

      w_ca_add_PointLight(&g_light_data.point_light_ca, &pl);
    }
  }

  /// limit fps to match the main.h macro defined value.
  // Variables for controlling FPS
  clock_t last_time = clock();
  double fps_timer = 0.0;

  char *cubemap_paths[6] = {TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png"),
                            TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png"),
                            TEXTURE_PATH("sky.png"), TEXTURE_PATH("sky.png")};

  // setup textures
  TextureHandle nepeta = textures[g_load_texture(TEXTURE_PATH("nepeta.jpg"))];
  TextureHandle character =
      textures[g_load_texture(TEXTURE_PATH("character.png"))];
  TextureHandle skybox_tex = textures[g_load_cubemap(cubemap_paths)];

  GraphicsRender *skybox_render = glprim_skybox_cube();
  skybox_render->pc = PC_SKYBOX;

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
    anim_update();

    hud_update();

    l_begin_draw();

    // after admin matrix stuff is over with, actually draw all the Renders.

    { // draw the 3d scene
      // our top-level, default pipeline.
      { // render skybox
        g_use_cubemap(skybox_tex);
        g_draw_render(skybox_render);
      }
      g_use_texture(nepeta);
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
  anim_clean();

  // LAST, clean up the main function in the backend.
  if (!l_clean()) {
    // error cleaning
    return 1;
  }

  return 0;
}
