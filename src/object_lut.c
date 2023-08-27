#include "object_lut.h"

#include "event_types.h"
#include "object.h"
#include "objects/camera.h"
#include "objects/character.h"
#include "objects/cube.h"
#include "objects/floor.h"
#include "objects/player.h"
#include "objects/sphere.h"

// do-nothing table functions. used as placeholders.
void none(void *v) {}
InteractionResponse none_int(void *v, InteractionEvent e) {
  return (InteractionResponse){
      IRT_NO_HANDLER}; // this object doesn't support interactions.
}

// INIT, UPDATE, CLEAN

// define the actual LUT object.
ObjectFnPointers fn_lut[OBJ_COUNT] = {
    [OBJ_PLAYER] = {player_init, player_update, player_clean},
    [OBJ_CAMERA] = {camera_init, camera_update, camera_clean},
    [OBJ_CUBE] = {cube_init, cube_update, cube_clean},
    [OBJ_SPHERE] = {sphere_init, sphere_update, sphere_clean},
    [OBJ_FLOOR] = {floor_init, floor_update, floor_clean},
    [OBJ_CHARACTER] = {character_init, character_update, character_clean},
};
