#include "ansi.h"
#include <stdio.h>
#include <stdlib.h>

#define IS_BETWEEN(a, min, max) ((a >= min) ? ((a < max) ? 1 : 0) : 0)

#define TIMES(code, num_times)                                                 \
  {                                                                            \
    int i_times_block;                                                         \
    while (i_times_block < num_times) {                                        \
      {code} i_times_block++;                                                  \
    }                                                                          \
  }

#define LOG(stream, color, severity_literal, msg, ...)                         \
  {                                                                            \
    fprintf(stream,                                                            \
            color severity_literal msg                                         \
            "\n\t[at file: (%s, %d); in: %s]" ANSI_RESET "\n",                 \
            ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__);           \
  }

#define INTERNAL_ERROR_MSG(msg, ...)                                           \
  LOG(stderr, ANSI_RED, "ERROR: ", msg, ##__VA_ARGS__)

#define INTERNAL_ERROR_MSG_NO_ARGS(msg) LOG(stderr, ANSI_RED, "ERROR: ", msg)

#define INFO(msg, ...)                                                         \
  { LOG(stdout, ANSI_BLUE, "INFO: ", msg, ##__VA_ARGS__); }

// the same thing as ERROR, just don't crash in debug.
#define WARNING(msg, ...)                                                      \
  { LOG(stderr, ANSI_YELLOW, "WARNING: ", msg, ##__VA_ARGS__); }

#ifdef DEBUG
#define ERROR(msg, ...)                                                        \
  {                                                                            \
    INTERNAL_ERROR_MSG(msg, __VA_ARGS__);                                      \
    fprintf(stderr, "Crashing...\n");                                          \
    exit(1);                                                                   \
  }
#define ERROR_NO_ARGS(msg)                                                     \
  {                                                                            \
    INTERNAL_ERROR_MSG_NO_ARGS(msg);                                           \
    fprintf(stderr, "Crashing...\n");                                          \
    exit(1);                                                                   \
  }
#else
#define ERROR(msg, ...)                                                        \
  { INTERNAL_ERROR_MSG(msg, __VA_ARGS__); }
#define ERROR_NO_ARGS(msg)                                                     \
  { INTERNAL_ERROR_MSG_NO_ARGS(msg); }
#endif // DEBUG

#define ERROR_FROM_BUF(message_buffer)                                         \
  { ERROR("%s", message_buffer); }

#define NULL_CHECK(value)                                                      \
  {                                                                            \
    if (value == NULL) {                                                       \
      ERROR_NO_ARGS("NULL check of " #value " failed.");                       \
    }                                                                          \
  }

#define NULL_CHECK_MSG(value, msg)                                             \
  {                                                                            \
    if (value == NULL) {                                                       \
      ERROR_NO_ARGS("NULL check of " #value " failed: " msg ".");              \
    }                                                                          \
  }

#define NULL_CHECK_STR_MSG(value, msg)                                         \
  {                                                                            \
    if (value == NULL) {                                                       \
      ERROR("NULL check of " #value " failed: %s.", msg);                      \
    }                                                                          \
  }

#define RUNTIME_ASSERT(expr)                                                   \
  {                                                                            \
    if (!(expr)) {                                                             \
      ERROR_NO_ARGS("assertion of " #expr " failed.");                         \
    }                                                                          \
  }

#define RUNTIME_ASSERT_MSG(expr, msg)                                          \
  {                                                                            \
    if (!(expr)) {                                                             \
      ERROR_NO_ARGS("assertion of " #expr " failed: " msg ".");                \
    }                                                                          \
  }

#define RUNTIME_ASSERT_STR_MSG(expr, msg)                                      \
  {                                                                            \
    if (!(expr)) {                                                             \
      ERROR("assertion of " #expr " failed: %s.", msg);                        \
    }                                                                          \
  }

#define NOT_IMPLEMENTED                                                        \
  fprintf(stderr, "ERROR: %s not implemented.\n", __PRETTY_FUNCTION__)
