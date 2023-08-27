#include "ansi.h"
#include <stdio.h>

// The "base case" to stop the recursion
#define DO_FOR_EACH_LAST(FUNC, x) FUNC(x)

// This does the recursion
#define DO_FOR_EACH(FUNC, x, ...)                                              \
  FUNC(x);                                                                     \
  DO_FOR_EACH_1(FUNC, __VA_ARGS__)

// To deal with the macro expansion order
#define DO_FOR_EACH_1(FUNC, ...) DO_FOR_EACH(FUNC, __VA_ARGS__)

#define TIMES(code, num_times)                                                 \
  {                                                                            \
    int i_times_block;                                                         \
    while (i_times_block < num_times) {                                        \
      {code} i_times_block++;                                                  \
    }                                                                          \
  }

#define INTERNAL_ERROR_MSG(msg)                                                \
  {                                                                            \
    fprintf(stderr,                                                            \
            ANSI_RED "ERROR: " msg                                             \
                     "\n\t[at file: (%s, %d); in: %s]" ANSI_RESET "\n",        \
            __FILE__, __LINE__, __PRETTY_FUNCTION__);                          \
  }

#ifdef DEBUG
#define ERROR(msg)                                                             \
  {                                                                            \
    INTERNAL_ERROR_MSG(msg);                                                   \
    fprintf(stderr, "Crashing...\n");                                          \
    exit(1);                                                                   \
  }
#else
#define ERROR(msg)                                                             \
  { INTERNAL_ERROR_MSG(msg); }
#endif // DEBUG

#define NULL_CHECK(value)                                                      \
  {                                                                            \
    if (value == NULL) {                                                       \
      ERROR("NULL check of " #value " failed.");                               \
    }                                                                          \
  }

#define NOT_IMPLEMENTED                                                        \
  fprintf(stderr, "ERROR: %s not implemented.\n", __PRETTY_FUNCTION__)
