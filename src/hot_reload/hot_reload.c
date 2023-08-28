#include <stdio.h>
#include <stdlib.h>

#include "macros.h"

#ifdef __linux__
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

static void watch_file_for_changes(const char *file_path) {
  char buffer[BUF_LEN];
  int file_descriptor = inotify_init();

  if (file_descriptor < 0) {
    perror("inotify_init");
    exit(EXIT_FAILURE);
  }

  int watch_descriptor =
      inotify_add_watch(file_descriptor, file_path, IN_MODIFY);

  if (watch_descriptor < 0) {
    perror("inotify_add_watch");
    exit(EXIT_FAILURE);
  }

  while (1) {
    ssize_t length = read(file_descriptor, buffer, BUF_LEN);

    if (length < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    struct inotify_event *event = (struct inotify_event *)buffer;

    if (event->mask & IN_MODIFY) {
      printf("File %s has been modified.\n", file_path);
    }
  }
}

static void *hot_reload_main(void *data) {
  printf("Starting up hot reload.\n");
  return NULL;
}
#else
static void *hot_reload_main(void *data) {
  printf("WARNING: hot reloading is only implemented for linux systems.\n");
  return NULL;
}
#endif

#include <pthread.h>

static pthread_t hr_thread;

void hot_reload_init() {
  if (pthread_create(&hr_thread, NULL, hot_reload_main, NULL)) {
    ERROR_NO_ARGS("Failed to create hot reload thread.\n");
  }
}

void hot_reload_clean() {
  // it's fine to join a thread even if it's been completed, just don't double
  // join a thread. that's UB
  if (pthread_join(hr_thread, NULL)) {
    ERROR_NO_ARGS("Failed to join hot reload thread.\n");
  }
}
