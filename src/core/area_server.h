#pragma once

#include "cglm/types.h"
#include "core/area_defines.h"
#include "objects/obj_area.h"
#include "parsers/area.h"
#include <stdbool.h>

typedef struct AreaState {
  AreaFile *area_file; // the parsed area from the const char path passed into
                       // the server by function call

  Position player_position;

  AreaObj
      *object_rep; // the physical representation of the area in the game world.
} AreaState;

extern AreaState area_state;

int area_init(const char *area_path);
int area_switch(const char *area_path);
// print out a 2d representation of the game map with the current player
// position.
void area_print_2d_rep(Position position);
bool area_will_collide(Position position);
bool area_player_move(Position position);
// use the same game-space world-space translation for everything, do it right
// here through this one function.
void area_get_vec3_from_position(Position position, vec3 dest);
// safely turn the Position local position storage into a vec3 that can be used
// for comparisons and geometry stuff.
void area_get_player_vec3(vec3 dest);
void area_interact(InteractionEvent e, InteractionResponse *responses);
