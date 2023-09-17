#include "area_server.h"
#include "macros.h"
#include "object.h"
#include "object_bases.h"
#include "os/library.h"
#include "path.h"
#include "whisper/colmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AreaEntry curr = {0};
// the path to the last area, for internal reloading.
char area_path_buf[512] = {0};

void area_switch(const char *path) {
  INFO("Switching area to %s.", path);

  object_clear_tag(OT_AREA);
  if (curr.cleaner)
    curr.cleaner();

  strcpy(area_path_buf, path);

  load_lib(&curr.lib, path);

  if (!curr.lib.handle) {
    ERROR("No area called %s found in libraries.", path);
    return;
  } else {
    // will just return NULL if any methods aren't found.
    curr.generator = lib_get_proc(&curr.lib, "init");
    curr.updater = lib_get_proc(&curr.lib, "update");
    curr.cleaner = lib_get_proc(&curr.lib, "clean");

    // then load in the new area, and setup the area_state.
    // generate the area in the game world automatically through the linked
    // function pointer.
    if (curr.generator)
      curr.generator();
  }
}

void area_init() { make_lib(&curr.lib); }

void area_clean() { lib_free(&curr.lib); }

void area_reload_curr() {
  area_switch(area_path_buf);
  INFO("Reloaded area %s.", area_path_buf);
}

#ifdef AREA_HOT_RELOAD

#include "hot_reload/hot_reload.h"
#include "macros.h"
#include "whisper/queue.h"
#include <pthread.h>

const char *base_area_path = "src/areas/instances";

static void recompile_area(char *file_name) {
  char buf[512];
  sprintf(buf,
          "gcc -o %s/%s.so -shared -fPIC %s/%s -Isrc -Ideps/libwhisper/api",
          base_area_path, file_name, base_area_path, file_name);
  printf("Running compilation command '%s'\n", buf);
  system(buf);
}

static void handle_hot_reload() {
  pthread_mutex_lock(hot_reload_state.mutex);

  WQueue *q = &(hot_reload_state.area_reload_events);

  for (;;) {
    AreaReloadEvent *a = w_dequeue(q);
    if (!a)
      break;

    // AreaEntry *entry = w_cm_get(&(areas), a->file_name);
    // if (!entry) {
    //   pthread_mutex_unlock(hot_reload_state.mutex);
    //   char buf[256];
    //   sprintf(buf, "Tried to reload [a->file_name: %s], no area found",
    //           a->file_name);
    //   ERROR_FROM_BUF(buf);
    // } else {
    //   printf("loading level %s\n", a->file_name);
    //   recompile_area(a->file_name);
    //   area_switch_areaentry(entry);
    // }
  }

  pthread_mutex_unlock(hot_reload_state.mutex);
}

#endif

void area_update() {
  if (curr.updater)
    curr.updater();

#ifdef AREA_HOT_RELOAD
  handle_hot_reload();
#endif
}
