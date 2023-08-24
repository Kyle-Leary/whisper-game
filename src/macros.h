#include <stdio.h>

#define TIMES(code, num_times)                                                 \
  {                                                                            \
    int i_times_block;                                                         \
    while (i_times_block < num_times) {                                        \
      {code} i_times_block++;                                                  \
    }                                                                          \
  }

#define NOT_IMPLEMENTED                                                        \
  fprintf(stderr, "ERROR: %s not implemented.\n", __PRETTY_FUNCTION__)
