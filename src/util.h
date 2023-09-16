#pragma once

#include "whisper/macros.h"
#include <errno.h>
#include <limits.h>

#include "macros.h"

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

// Function to get the length of the file
size_t get_file_length(const char *file_path);
// stack return values are for chumps. pass in an address and fill it with the
// data in the function.
void read_file_data(const char *file_path, char *return_value, int buffer_len);

extern const char *str_to_int_underflow;
extern const char *str_to_int_invalid_int;
extern const char *str_to_int_out_of_range;

// define this in a macro, since we want the error to show the caller's line and
// function.
#define STR_TO_INT(str)                                                        \
  int str_to_int_return = 0;                                                   \
  char *err_ptr = NULL;                                                        \
  {                                                                            \
    char *endptr;                                                              \
    errno = 0;                                                                 \
    long val = strtol(str, &endptr, 10);                                       \
    if (errno == ERANGE) {                                                     \
      err_ptr = (char *)str_to_int_underflow;                                  \
    } else if (endptr == str || *endptr != '\0') {                             \
      err_ptr = (char *)str_to_int_invalid_int;                                \
    } else if (val > INT_MAX || val < INT_MIN) {                               \
      err_ptr = (char *)str_to_int_out_of_range;                               \
    }                                                                          \
    str_to_int_return = (int)val;                                              \
  }
