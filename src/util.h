#pragma once

#include <stdio.h>
#include <stdlib.h>

#define PRINT_PTR(ptr)                                                         \
  { printf("ptr " #ptr ": %p\n", ptr); }

#define PRINT_INT(int_val)                                                     \
  { printf("int " #int_val ": %d\n", int_val); }

#define PRINT_STRING(str_val)                                                  \
  { printf("string " #str_val ": %s\n", str_val); }

#define PRINT_DOUBLE(double_val)                                               \
  { printf("double " #double_val ": %f\n", double_val); }

#define PRINT_FLOAT(float_val)                                                 \
  { printf("float " #float_val ": %f\n", float_val); }

// used for debugging, figuring out where a program segfaults by exiting.
#define STOP                                                                   \
  {                                                                            \
    printf("Force exit with STOP at: %d, %s\n", __LINE__, __FILE__);           \
    exit(0);                                                                   \
  }

// Function to get the length of the file
size_t get_file_length(const char *file_path);
// stack return values are for chumps. pass in an address and fill it with the
// data in the function.
void read_file_data(const char *file_path, char *return_value, int buffer_len);
