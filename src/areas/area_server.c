#include "area_server.h"
#include "object.h"
#include "object_bases.h"
#include "path.h"
#include "whisper/colmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

WColMap areas = {0};
AreaEntry *curr = NULL;

static void area_switch_areaentry(AreaEntry *area) {
  if (curr) { // remove all the old area stuff generically.
    object_clear_tag(OT_AREA);
    if (curr->cleaner)
      curr->cleaner();
  }

  curr = area;

  { // then load in the new area, and setup the area_state.
    // generate the area in the game world automatically through the linked
    // function pointer.
    if (curr->generator)
      curr->generator();
  }
}

void area_switch(const char *path) {
  printf("Switching area to %s.\n", path);

  AreaEntry *ae = w_cm_get(&areas, path);
  if (!ae) {
    fprintf(stderr, "No area called %s is registered.\n", path);
    return;
  } else {
    area_switch_areaentry(ae);
  }
}

void areas_level();
void areas_level_update();

void areas_static();
void areas_static_update();

void areas_bone_test();
void areas_bone_test_update();

void areas_simple_physics();
void areas_simple_physics_update();

void areas_animation();
void areas_animation_update();

void areas_gui();
void areas_gui_update();

void areas_video();
void areas_video_update();

void areas_another();

void area_init() {
  // hash the names of the .c files that represent the areas to get the location
  // in the colmap.
  w_create_cm(&(areas), sizeof(AreaEntry), 509);

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "level.c");
    memcpy(area, &(AreaEntry){areas_level, areas_level_update, NULL},
           sizeof(AreaEntry));
  }

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "static.c");
    memcpy(area, &(AreaEntry){areas_static, areas_static_update, NULL},
           sizeof(AreaEntry));
  }

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "another.c");
    memcpy(area, &(AreaEntry){areas_another, NULL, NULL}, sizeof(AreaEntry));
  }

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "bone_test.c");
    memcpy(area, &(AreaEntry){areas_bone_test, areas_bone_test_update, NULL},
           sizeof(AreaEntry));
  }

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "simple_physics.c");
    memcpy(
        area,
        &(AreaEntry){areas_simple_physics, areas_simple_physics_update, NULL},
        sizeof(AreaEntry));
  }

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "animation.c");
    memcpy(area, &(AreaEntry){areas_animation, areas_animation_update, NULL},
           sizeof(AreaEntry));
  }

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "gui.c");
    memcpy(area, &(AreaEntry){areas_gui, areas_gui_update, NULL},
           sizeof(AreaEntry));
  }

  {
    AreaEntry *area = w_cm_return_slot(&(areas), "video.c");
    memcpy(area, &(AreaEntry){areas_video, areas_video_update, NULL},
           sizeof(AreaEntry));
  }
}

void area_clean() { w_free_cm(&areas); }

void area_reload_curr() {
  if (curr)
    area_switch_areaentry(curr);
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

    AreaEntry *entry = w_cm_get(&(areas), a->file_name);
    if (!entry) {
      pthread_mutex_unlock(hot_reload_state.mutex);
      char buf[256];
      sprintf(buf, "Tried to reload [a->file_name: %s], no area found",
              a->file_name);
      ERROR_FROM_BUF(buf);
    } else {
      printf("loading level %s\n", a->file_name);
      recompile_area(a->file_name);
      area_switch_areaentry(entry);
    }
  }

  pthread_mutex_unlock(hot_reload_state.mutex);
}

#endif

void area_update() {
  // update the current area
  if (curr && curr->updater)
    curr->updater();

#ifdef AREA_HOT_RELOAD
  handle_hot_reload();
#endif
}
