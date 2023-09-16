#include "event_types.h"
#include "meshing/gltf_mesher.h"
#include "object_bases.h"
#include "objtypedef.h"
#include "path.h"
#include "physics/body/body.h"
#include <stdint.h>
#include <stdio.h>

void areas_another() {
  Player *player = (Player *)object_add((Object *)player_build(), OT_AREA);
  player->phys->body->position[0] = 5;
  player->phys->body->position[1] = -1;

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build(
          (vec3){0}, &(((RigidBody *)player->phys->body)->lerp_position)),
      OT_AREA);

  object_add((Object *)floor_build((vec3){0, -1, 0}), OT_AREA);
}
