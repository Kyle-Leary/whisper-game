#pragma once

#include "cglm/types.h"

#include <stdbool.h>

typedef void (*AreaFn)();
typedef void (*AreaUpdateFn)();
typedef void (*AreaCleanFn)();

typedef struct AreaEntry {
  // each game area/level has its own arbitrary lifecycle methods.
  AreaFn generator;
  AreaUpdateFn updater;
  AreaCleanFn cleaner;
} AreaEntry;

void area_init();
void area_clean();
void area_switch(const char *path);
void area_reload_curr();
void area_update();
