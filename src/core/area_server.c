#include "area_server.h"
#include "areas/areas.h"
#include "object.h"
#include "object_bases.h"
#include <stdio.h>

AreaState area_state = {0};

static void none_area() {}

static AreaEntry areas[AREA_COUNT] = {
    [AREA_INVALID] = {none_area, none_area, none_area},

    [AREA_LEVEL] = {areas_level, areas_level_update, none_area},
    [AREA_ANOTHER] = {areas_another, none_area, none_area},
};

void area_switch(AreaID id) {
  printf("Switching area to %d.\n", id);

  { // remove all the stuff from the old area
    object_clear_tag(OT_AREA);
    areas[area_state.curr].cleaner();
  }

  { // then load in the new area, and setup the area_state.
    area_state.curr = id;

    // generate the area in the game world automatically through the linked
    // function pointer.
    areas[area_state.curr].generator();
  }
}

void area_update() { areas[area_state.curr].updater(); }
