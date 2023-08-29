#pragma once

#include "whisper/queue.h"
#include <bits/pthreadtypes.h>

typedef struct AreaReloadEvent {
  char file_name[256];
} AreaReloadEvent;

typedef struct HotReloadState {
  WQueue area_reload_events;
  pthread_mutex_t *mutex;
} HotReloadState;

extern HotReloadState hot_reload_state;

void hot_reload_init();
void hot_reload_clean();
