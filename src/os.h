#pragma once

// macros and functions for simple cross-platform OS functionality.

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
typedef LARGE_INTEGER whisper_time_t;
#define GET_TIME(counter) QueryPerformanceCounter(&counter)
#define TIME_DIFF_SECONDS(start, end)                                          \
  ((double)(end.QuadPart - start.QuadPart) / frequency.QuadPart)
#define TIME_INIT()                                                            \
  LARGE_INTEGER frequency;                                                     \
  QueryPerformanceFrequency(&frequency)

#elif defined(__linux__)
#include <time.h>
typedef struct timespec whisper_time_t;
#define GET_TIME(counter) clock_gettime(CLOCK_MONOTONIC_RAW, &counter)
#define TIME_DIFF_SECONDS(start, end)                                          \
  (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1e9)
#define TIME_INIT()

#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
typedef uint64_t whisper_time_t;
#define GET_TIME(counter) counter = mach_absolute_time()
#define TIME_DIFF_SECONDS(start, end)                                          \
  ((double)(end - start) * timebase.numer / timebase.denom / 1e9)
#define TIME_INIT()                                                            \
  mach_timebase_info_data_t timebase;                                          \
  mach_timebase_info(&timebase)

#endif

void os_init();
// what should each thread do on startup?
void os_thread_init();
void musleep(int ms);
