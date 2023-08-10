#pragma once

#include <stdio.h>
#include <stdlib.h>

#define PRINT_PTR(ptr)                                                         \
  { printf("ptr " #ptr ": %p\n", ptr); }

// Function to get the length of the file
size_t get_file_length(const char *file_path);
// stack return values are for chumps. pass in an address and fill it with the
// data in the function.
void read_file_data(const char *file_path, char *return_value, int buffer_len);
