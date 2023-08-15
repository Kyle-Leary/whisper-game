#include "areas.h"
#include "core/area_server.h"
#include "event_types.h"
#include "meshing/gltf_mesher.h"
#include "object_bases.h"
#include "objtypedef.h"
#include "path.h"
#include <stdint.h>
#include <stdio.h>

#include "backends/graphics_api.h"

// can also handle light logic in the area object definitions.
static uint16_t player_id;

void handle_area_enter(void *p, CollisionEvent *e) {
  if (e->magnitude > 0.01F && e->id == player_id) {
    // the player is exerting some non-zero force onto the entity.
    area_switch(AREA_ANOTHER);
  }
}

// setup all the local objects in the scene.
void areas_level() {
  Player *player = (Player *)object_add((Object *)player_build(), OT_AREA);
  player->position[0] = 5;
  player->position[1] = -1;

  player_id = player->id;

  // note that the camera is directly LINKED to the player's lerp_position from
  // its physics representation.
  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &player->lerp_position), OT_AREA);

  Character *chr =
      (Character *)object_add((Object *)character_build(gltf_to_model(
                                  gltf_parse(MODEL_PATH("suzanne.glb")))),
                              OT_AREA);
  chr->position[0] = -4;
  chr->position[1] = -1;

  {
    Collider c;
    c.type = CL_SPHERE;
    SphereColliderData *col_data =
        (SphereColliderData *)malloc(sizeof(SphereColliderData) * 1);
    col_data->radius = 1;
    c.data = col_data;

    Detector *d = (Detector *)object_add(
        (Object *)detector_build((vec3){0}, &c, handle_area_enter), OT_AREA);
  }

  object_add((Object *)cube_build((vec3){9, -1, 0}), OT_AREA);
  object_add((Object *)cube_build((vec3){0, -1, 9}), OT_AREA);
  object_add((Object *)cube_build((vec3){0, -1, -9}), OT_AREA);
  object_add((Object *)cube_build((vec3){-9, -1, -9}), OT_AREA);
  object_add((Object *)cube_build((vec3){7, -1, -9}), OT_AREA);

  object_add((Object *)sphere_build((vec3){2, -1, -2}, 1, 10), OT_AREA);

  object_add((Object *)floor_build((vec3){0, -1, 0}, 50), OT_AREA);

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
      pl.position[1] = 0.0f;
      pl.position[2] = 0.0f;
      pl.color[0] = 0.5f;
      pl.color[1] = 0.1f;
      pl.color[2] = 0.5f;
      pl.color[3] = 1.0f;

      w_ca_add_PointLight(&g_light_data.point_light_ca, &pl);
    }
  }
}
