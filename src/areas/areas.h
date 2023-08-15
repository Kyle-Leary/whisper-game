#pragma once

typedef void (*AreaFn)();

// select areas by enum.
typedef enum AreaID {
  AREA_INVALID = 0,
  AREA_LEVEL,
  AREA_ANOTHER,

  AREA_COUNT,
} AreaID;

void areas_level();
void areas_another();
