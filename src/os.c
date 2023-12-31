#include "os.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <execinfo.h>
#include <signal.h>
#endif

void musleep(int milliseconds) {
#ifdef _WIN32
  Sleep(milliseconds);
#elif defined(__unix__)
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
#else
#error "Unknown compiler, cannot determine how to sleep"
#endif
}

#ifdef __unix__
void sigsegv(int sig) {
  void *array[25];
  size_t size;

  // Get void*'s for all entries on the stack
  size = backtrace(array, 25);

  fprintf(stderr, "\n\nSEGFAULT sig %d: stacktrace - \n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);

  exit(1);
}
#else
#endif

void os_init() {
  os_thread_init();

#ifdef __unix__
#else
#endif
}

void os_thread_init() {
#ifdef __unix__
  // each thread needs to independently set up its signal handlers.
  signal(SIGSEGV, sigsegv);
#else
#endif
}
