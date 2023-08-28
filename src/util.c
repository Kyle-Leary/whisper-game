#include "util.h"
#include "helper_math.h"
#include "pragma.h"

const char *str_to_int_underflow = "Error: Integer overflow or underflow.\n";
const char *str_to_int_invalid_int = "Error: Not a valid integer.\n";
const char *str_to_int_out_of_range = "Error: Out of `int` range.\n";

size_t get_FILE_length(FILE *file) {
  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
  fseek(file, 0, SEEK_SET);
  return length;
}

// Function to get the length of the file
size_t get_file_length(const char *file_path) {
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    perror("Error opening file");
    return 0;
  }

  return get_FILE_length(file);
}

void read_file_data(const char *file_path, char *return_value, int buffer_len) {
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    perror("Error opening file");
    return;
  }

  size_t min = MIN(buffer_len, get_FILE_length(file));
  fread(return_value, 1, min, file);
  return_value[min] = '\0'; // then, safely null-terminate the data read in.
  fclose(file);
}
