#define TIMES(code, num_times)                                                 \
  {                                                                            \
    int i_times_block;                                                         \
    while (i_times_block < num_times) {                                        \
      {code} i_times_block++;                                                  \
    }                                                                          \
  }
