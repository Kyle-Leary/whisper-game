#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: pthread is cross-plat, inotify file listeners are not.

#include "helper_math.h"
#include "hot_reload/hot_reload.h"
#include "macros.h"
#include "os.h"
#include "whisper/macros.h"
#include "whisper/queue.h"

#include <pthread.h>

#define MAX_AREA_RELOAD_EVENTS 16

HotReloadState hot_reload_state = {0};

#ifdef AREA_HOT_RELOAD
#include <dirent.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static void *handle = NULL;

void reload_areas() {
  if (handle) {
    // unload the shared library if it's already open.
    if (dlclose(handle) != 0) {
      fprintf(stderr, "dlclose failed: %s\n", dlerror());
      exit(EXIT_FAILURE);
    }
  }

  int (*some_function)(int);

  // Load the shared library
  handle = dlopen("./libsome.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen failed: %s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  // Clear any existing error
  dlerror();

  // Get pointer to the function
  some_function = (int (*)(int))dlsym(handle, "some_function");
  char *error = dlerror();
  if (error != NULL) {
    fprintf(stderr, "dlsym failed: %s\n", error);
    exit(EXIT_FAILURE);
  }
}

#include <stdio.h>
#include <stdlib.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

static bool is_thread_running = true;

#define MAX_LISTENERS 32

static int wds[MAX_LISTENERS] = {0};
static char wd_basenames[MAX_LISTENERS][256] = {0};
static int num_wds = 0;

static void watch_dir_for_changes(const char *dir_path, HotReloadState *state) {
  char buffer[BUF_LEN];
  char filepath_buf[256];
  int file_descriptor = inotify_init();

  if (file_descriptor < 0) {
    perror("inotify_init");
    exit(EXIT_FAILURE);
  }

  {
    DIR *dir = opendir(dir_path);

    if (dir == NULL) {
      perror("Failed to open directory");
      exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    int num_files = 0;
    while ((entry = readdir(dir)) != NULL) {
      // Skip the entries "." and ".." as we don't want to loop infinitely
      if (entry->d_name[0] == '.')
        continue;

      int len = strlen(entry->d_name);
      if (entry->d_name[len - 1] == 'c' && entry->d_name[len - 2] == '.') {
        // sprintf null terms, this is okay.
        sprintf(filepath_buf, "%s/%s", dir_path,
                entry->d_name); // we need both the abspath and the basename.
        memcpy(wd_basenames[num_wds], entry->d_name, len);

        wds[num_wds] =
            inotify_add_watch(file_descriptor, filepath_buf, IN_MODIFY);

        if (wds[num_wds] < 0) {
          perror("inotify_add_watch");
          exit(EXIT_FAILURE);
        }

        num_files++;
        num_wds++;

        printf("Listening to area '%s'\n", filepath_buf);

        if (num_wds >= MAX_LISTENERS) {
          ERROR_NO_ARGS("too many files, increase MAX_LISTENERS.");
        }
      }
    }

    closedir(dir);
  }

  while (1) {
    if (!is_thread_running)
      return;

    ssize_t length = read(file_descriptor, buffer, BUF_LEN);

    if (length < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    // pop through all the events, the read syscall can flush several events at
    // a time into the file, and we have to catch up on each pass.
    char *ptr;
    struct inotify_event *event;
    for (ptr = buffer; ptr < buffer + length;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (struct inotify_event *)ptr;

      if (event->mask & IN_MODIFY) {
        AreaReloadEvent a = {0};
        for (int i = 0; i < MAX_LISTENERS; i++) {
          if (wds[i] == event->wd) {
            strcpy(a.file_name, wd_basenames[i]);
            break;
          }
        }
        printf("Modified area '%s'\n", a.file_name);
        pthread_mutex_lock(state->mutex);
        w_enqueue(&(state->area_reload_events), &a);
        pthread_mutex_unlock(state->mutex);
      }
    }
  }
}

static void *hot_reload_main(void *data) {
  os_thread_init();

  HotReloadState *state = (HotReloadState *)data;
  printf("Starting up hot reload.\n");

  // watch_dir_for_changes("./src/areas/instances", data);

  return NULL;
}
#else
static void *hot_reload_main(void *data) {
  printf("WARNING: hot reloading is only implemented for linux systems.\n");
  return NULL;
}
#endif

static pthread_t hr_thread;

void hot_reload_init() {
  w_make_queue(&(hot_reload_state.area_reload_events), sizeof(AreaReloadEvent),
               MAX_AREA_RELOAD_EVENTS);

  { // init the HotReloadState pthread mutex.
    pthread_mutex_t *mutex_ptr =
        (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (mutex_ptr == NULL) {
      fprintf(stderr, "Failed to allocate memory for mutex.\n");
      exit(EXIT_FAILURE);
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    if (pthread_mutex_init(mutex_ptr, &attr) != 0) {
      fprintf(stderr, "Failed to initialize mutex.\n");
      free(mutex_ptr);
      exit(EXIT_FAILURE);
    }

    hot_reload_state.mutex = mutex_ptr;
  }

  if (pthread_create(&hr_thread, NULL, hot_reload_main, &hot_reload_state)) {
    ERROR_NO_ARGS("Failed to create hot reload thread.\n");
  }
}

void hot_reload_clean() {
  is_thread_running = false;

  // there's no easy way to stop the inotify read from blocking the thread, and
  // once that happens "joining" requires us to wait until it's finished. just
  // kill the thread for now?
  if (pthread_cancel(hr_thread)) {
    ERROR_NO_ARGS("Failed to cancel hot reload thread.\n");
  }

  free(hot_reload_state.mutex);
}
