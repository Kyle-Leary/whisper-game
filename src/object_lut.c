#include "object_lut.h"

#include "event_types.h"
#include "object.h"
#include "objects/button.h"
#include "objects/camera.h"
#include "objects/character.h"
#include "objects/cube.h"
#include "objects/floor.h"
#include "objects/label.h"
#include "objects/obj_area.h"
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

// INIT, UPDATE, DRAW, DRAW_UI, CLEAN, HANDLE_COLLISION

// define the actual LUT object.
ObjectFnPointers fn_lut[OBJ_COUNT] = {
    // for example, we'll do fn_lut[o->type].update(o); to call the update
    // function. minimize icache misses by deferring behavior to different
    // functions, rather than inlining in a huge switch.
    [OBJ_RENDER] = {none, none, render_draw, none, render_clean, none_col,
                    none_int},
    [OBJ_LABEL] = {label_init, label_update, none, label_draw_hud, label_clean,
                   none_col, none_int},
    [OBJ_BUTTON] = {button_init, button_update, none, button_draw_hud,
                    button_clean, none_col, none_int},
    [OBJ_TEXTURE] = {texture_init, texture_update, none, texture_draw_hud,
                     texture_clean, none_col, none_int},

    [OBJ_PLAYER] = {player_init, player_update, player_draw, none, player_clean,
                    player_handle_collision,
                    none_int}, // register event handler functions here too.
    [OBJ_CAMERA] = {camera_init, camera_update, none, none, camera_clean,
                    camera_handle_collision,
                    none_int}, // register event handler functions here too.
    [OBJ_CUBE] = {cube_init, cube_update, cube_draw, none, cube_clean,
                  cube_handle_collision,
                  none_int}, // register event handler functions here too.
    [OBJ_SPHERE] = {sphere_init, sphere_update, sphere_draw, none, sphere_clean,
                    sphere_handle_collision,
                    none_int}, // register event handler functions here too.
    [OBJ_FLOOR] = {floor_init, floor_update, floor_draw, none, floor_clean,
                   floor_handle_collision,
                   none_int}, // register event handler functions here too.
    [OBJ_CHARACTER] = {character_init, character_update, character_draw, none,
                       character_clean, none_col,
                       character_handle_interact}, // register event handler
                                                   // functions here too.
    [OBJ_AREAOBJ] = {areaObj_init, areaObj_update, areaObj_draw, none,
                     areaObj_clean, none_col, none_int},
};
