#pragma once

#include "areas/areas.h"
#include "cglm/types.h"
#include "core/area_defines.h"
#include "whisper/hashmap.h"
#include <stdbool.h>

typedef struct AreaEntry {
  // each game area/level has its own arbitrary lifecycle methods.
  AreaFn generator;
  AreaUpdateFn updater;
  AreaCleanFn cleaner;
} AreaEntry;

typedef struct AreaState {
  AreaID curr;
} AreaState;

extern AreaState area_state;

void area_switch(AreaID id);
void area_update();
