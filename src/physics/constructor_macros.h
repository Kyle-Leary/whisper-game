#pragma once

#define INDEX_AND_RETURN(to_insert, which)                                     \
  {                                                                            \
    int idx = w_array_insert(&(physics_state.which), &to_insert);              \
    if (idx != -1) {                                                           \
      return w_array_get(&(physics_state.which), idx);                         \
    } else {                                                                   \
      fprintf(stderr, "Error: Too many " #which ".\n");                        \
      return NULL;                                                             \
    }                                                                          \
  }
