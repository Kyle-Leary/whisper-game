#include "animation/animator.h"
#include "global.h"
#include "gui/gui.h"
#include "meshing/font.h"
#include "meshing/gltf_mesher.h"
#include "object_bases.h"
#include "objtypedef.h"
#include "path.h"
#include <stdint.h>
#include <stdio.h>

#include "physics/body/body.h"
#include "physics/physics.h"
#include "printers.h"
#include "render/light.h"

// can also handle light logic in the area object definitions.
static uint16_t player_id;

static GraphicsRender *text_3d = NULL;
static Player *our_player = NULL;

void setup_text3d_render() {
  g_use_texture(simple_font->tex_handle, FONT_TEX_SLOT);
}

// setup all the local objects in the scene.
void areas_level() {
  object_add((Object *)sphere_build((vec3){3, 5, -2}, 1, 10), OT_AREA);
  object_add((Object *)sphere_build((vec3){6, 10, -2}, 1, 10), OT_AREA);
  object_add((Object *)sphere_build((vec3){-2, 20, -2}, 1, 10), OT_AREA);

  object_add((Object *)cube_build((vec3){9, 5, 0}, (vec3){1, 2, 8}), OT_AREA);
  object_add((Object *)cube_build((vec3){0, 5, 9}, (vec3){1, 2, 3}), OT_AREA);
  object_add((Object *)cube_build((vec3){0, 5, -9}, (vec3){1, 2, 4}), OT_AREA);
  object_add((Object *)cube_build((vec3){-9, 5, -9}, (vec3){1, 2, 1}), OT_AREA);
  object_add((Object *)cube_build((vec3){7, 5, -9}, (vec3){1, 2, 9}), OT_AREA);

  our_player = (Player *)object_add((Object *)player_build(), OT_AREA);
  our_player->phys->body->position[0] = 5;
  our_player->phys->body->position[1] = 1;

  // note that the camera is directly LINKED to the our_player's lerp_position
  // from its physics representation.
  Camera *cam = (Camera *)object_add(
      (Object *)camera_build(
          (vec3){0}, &(((RigidBody *)our_player->phys->body)->lerp_position)),
      OT_AREA);

  Model *chr_model = gltf_to_model(gltf_parse(MODEL_PATH("wiggle.glb")));

  Character *chr =
      (Character *)object_add((Object *)character_build(chr_model), OT_AREA);
  chr->phys->body->position[0] = -4;
  chr->phys->body->position[1] = -1;

  object_add((Object *)floor_build((vec3){0, 0, 0}), OT_AREA);

  {   // setup global lights
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
      pl.position[1] = 5.0f;
      pl.position[2] = 0.0f;
      pl.color[0] = 0.5f;
      pl.color[1] = 0.1f;
      pl.color[2] = 0.5f;
      pl.color[3] = 1.0f;

      w_ca_add_PointLight(&g_light_data.point_light_ca, &pl);
    }
  }

  text_3d = font_mesh_string_3d(simple_font, "hello 3d space", 0.5, 0.5);
}

void areas_level_update() {
  glm_rotate(text_3d->model, 0.01, (vec3){0, 1, 0});

  {}
}
