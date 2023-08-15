#pragma once

#include "areas/areas.h"
#include "cglm/types.h"
#include "core/area_defines.h"
#include "whisper/hashmap.h"
#include <stdbool.h>

typedef struct AreaEntry {
  AreaFn generator; // a function to construct the area.
} AreaEntry;

typedef struct AreaState {
} AreaState;

extern AreaState area_state;

void area_switch(AreaID id);
