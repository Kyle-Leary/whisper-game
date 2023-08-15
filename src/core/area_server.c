#include "area_server.h"
#include "areas/areas.h"
#include "object.h"
#include "object_bases.h"
#include <stdio.h>

AreaState area_state = {};

static AreaEntry areas[AREA_COUNT] = {
    [AREA_LEVEL] = {areas_level},
    [AREA_ANOTHER] = {areas_another},
};

void area_switch(AreaID id) {
  printf("Switching area to %d.\n", id);

  object_clear_tag(OT_AREA);

  // generate the area in the game world automatically through the linked
  // function pointer.
  areas[id].generator();
}
