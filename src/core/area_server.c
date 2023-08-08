#include "area_server.h"

#include "backends/audio_api.h"
#include "cglm/types.h"
#include "core/area_defines.h"
#include "core/tile.h"
#include "event_types.h"
#include "object.h"
#include "objects/obj_area.h"
#include "parsers/area.h"
#include "path.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

AreaState area_state;

int area_init(const char *area_path) { return area_switch(area_path); }

int area_switch(const char *area_path) {
  if (area_state.object_rep !=
      NULL) // will call the proper free method and remove it from the server.
    object_remove_by_ptr((Object *)area_state.object_rep);
  if (area_state.area_file !=
      NULL) // just free the file, we're not doing anything special with it.
    free(area_state.area_file);

  area_state.area_file = parse_area(AREA_PATH("larger_test.area"));
  if (!area_state.area_file) {
    printf("failed to open area! [from path: %s]\n", area_path);
    return 1;
  }

  printf("Switching area... [%s]\n", area_path);

  // then, generate the object for the area, put it into the object server.

  area_state.object_rep =
      (AreaObj *)object_add((Object *)areaObj_build(area_state.area_file));

  return 0;
}

void area_print_2d_rep(ivec2 position) {
  for (int i = 0; i < area_state.area_file->x_size; i++) {
    for (int j = 0; j < area_state.area_file->y_size; j++) {
      // if it's the player's position.
      if (position[0] == i && position[1] == j)
        printf(" P ");
      else
        printf(" %c ", tile_ch[area_state.area_file->tiledata[i][j].type]);
    }
    printf("\n");
  }
}

// given the passed position, will it be in a wall or something?
bool area_will_collide(Position position) {
  // we can't collide with anything out of bounds.
  if (position[0] < 0 || position[1] < 0)
    return false;
  TileData subject = area_state.area_file->tiledata[position[0]][position[1]];
  // printf("checking collision on (%d, %d)\n", position[0], position[1]);
  // print_tiledata(subject);
  switch (subject.type) {
  case TT_FLOOR:
    // it's a floor, not going to collide.
    return false;
    break;
  case TT_WALL:
    // bumping into a wall
    a_play_pcm(SOUND_PATH("wall_hit.wav"));
    break;
  default:
    break;
  }
  return true;
}

// basically the player interaction handler.
bool area_player_move(Position position) {
  // drop the case where we can't move, first of all.
  if (area_will_collide(position)) {
    return false;
  }

  // then handle object collisions and interactions, since we're IT_MOVE_INTOing
  // onto a new tile.
  InteractionResponse responses[NUM_OBJECTS] = {
      0}; // don't put bad data in the struct
  InteractionEvent sent_event = {IT_MOVE_INTO, position[0], position[1]};
  object_interact(
      sent_event,
      responses); // pass the object interaction handler the position we WANT to
                  // be in, BEFORE we move there.

  int is_moving = true;

  for (int i = 0; i < NUM_OBJECTS; i++) {
    switch (responses[i].type) {
    case IRT_BLOCK:
      is_moving = false;
      break;
    default:
      break;
    }
  }

  // only if we haven't been blocked by anything.
  if (is_moving)
    // then copy in the new position
    memcpy(area_state.player_position, position, sizeof(int) * 2);
  else
    return false; // still return false, couldn't move

  return true;
}

// again, pass in the response container of size NUM_OBJECTS.
// this is basically a wrapper around the object_interact function.
void area_interact(InteractionEvent e, InteractionResponse *responses) {
  object_interact(e, responses);
}

void area_get_vec3_from_position(Position position, vec3 dest) {
  dest[0] = position[1];
  dest[1] = 1.0F; // at an offset, so they're above the ground.
  dest[2] = position[0];
}

void area_get_player_vec3(vec3 dest) {
  area_get_vec3_from_position(area_state.player_position, dest);
}
