#pragma once

typedef void (*AreaFn)();
typedef void (*AreaUpdateFn)();
typedef void (*AreaCleanFn)();

// select areas by enum.
typedef enum AreaID {
  AREA_INVALID = 0,
  AREA_LEVEL,
  AREA_ANOTHER,
  AREA_STATIC,

  AREA_COUNT,
} AreaID;

void areas_level();
void areas_level_update();

void areas_static();
void areas_static_update();

void areas_another();
