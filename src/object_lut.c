#include "object_lut.h"

#include "event_types.h"
#include "object.h"
#include "objects/button.h"
#include "objects/camera.h"
#include "objects/character.h"
#include "objects/cube.h"
#include "objects/floor.h"
#include "objects/label.h"
#include "objects/player.h"
#include "objects/sphere.h"
#include "objects/texture.h"

// do-nothing table functions. used as placeholders.
void none(void *v) {}
InteractionResponse none_int(void *v, InteractionEvent e) {
  return (InteractionResponse){
      IRT_NO_HANDLER}; // this object doesn't support interactions.
}

// INIT, UPDATE, CLEAN

// define the actual LUT object.
ObjectFnPointers fn_lut[OBJ_COUNT] = {
    // for example, we'll do fn_lut[o->type].update(o); to call the update
    // function. minimize icache misses by deferring behavior to different
    // functions, rather than inlining in a huge switch.
    [OBJ_LABEL] = {label_init, label_update, label_clean, none_int},
    [OBJ_BUTTON] = {button_init, button_update, button_clean, none_int},
    [OBJ_TEXTURE] = {texture_init, texture_update, texture_clean, none_int},

    [OBJ_PLAYER] = {player_init, player_update, player_clean, none_int},
    [OBJ_CAMERA] = {camera_init, camera_update, camera_clean, none_int},
    [OBJ_CUBE] = {cube_init, cube_update, cube_clean, none_int},
    [OBJ_SPHERE] = {sphere_init, sphere_update, sphere_clean, none_int},
    [OBJ_FLOOR] = {floor_init, floor_update, floor_clean, none_int},
    [OBJ_CHARACTER] = {character_init, character_update, character_clean,
                       none_int},
};
