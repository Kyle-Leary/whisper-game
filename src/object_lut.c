#include "object_lut.h"

#include "event_types.h"
#include "object.h"
#include "objects/button.h"
#include "objects/camera.h"
#include "objects/character.h"
#include "objects/cube.h"
#include "objects/detector.h"
#include "objects/floor.h"
#include "objects/label.h"
#include "objects/player.h"
#include "objects/render.h"
#include "objects/sphere.h"
#include "objects/texture.h"

// do-nothing table functions. used as placeholders.
void none(void *v) {}
void none_col(void *v, CollisionEvent *e) {}
InteractionResponse none_int(void *v, InteractionEvent e) {
  return (InteractionResponse){
      IRT_NO_HANDLER}; // this object doesn't support interactions.
}

#define TODO                                                                   \
  { none, none, none, none, none, none_col, none_int }

// INIT, UPDATE, DRAW, CLEAN, HANDLE_COLLISION

// define the actual LUT object.
ObjectFnPointers fn_lut[OBJ_COUNT] = {
    // for example, we'll do fn_lut[o->type].update(o); to call the update
    // function. minimize icache misses by deferring behavior to different
    // functions, rather than inlining in a huge switch.
    [OBJ_RENDER] = {none, none, render_draw, render_clean, none_col, none_int},
    [OBJ_LABEL] = {label_init, label_update, label_draw, label_clean, none_col,
                   none_int},
    [OBJ_BUTTON] = {button_init, button_update, button_draw, button_clean,
                    none_col, none_int},
    [OBJ_TEXTURE] = {texture_init, texture_update, texture_draw, texture_clean,
                     none_col, none_int},

    [OBJ_PLAYER] = {player_init, player_update, player_draw, player_clean,
                    player_handle_collision, none_int},
    [OBJ_CAMERA] = {camera_init, camera_update, none, camera_clean,
                    camera_handle_collision, none_int},
    [OBJ_CUBE] = {cube_init, cube_update, cube_draw, cube_clean,
                  cube_handle_collision, none_int},
    [OBJ_SPHERE] = {sphere_init, sphere_update, sphere_draw, sphere_clean,
                    sphere_handle_collision, none_int},
    [OBJ_DETECTOR] = {detector_init, detector_update, none, detector_clean,
                      detector_handle_collision, none_int},
    [OBJ_FLOOR] = {floor_init, floor_update, floor_draw, floor_clean,
                   floor_handle_collision, none_int},
    [OBJ_CHARACTER] = {character_init, character_update, character_draw,
                       character_clean, none_col, character_handle_interact},
};
