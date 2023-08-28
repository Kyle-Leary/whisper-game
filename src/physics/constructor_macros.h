#pragma once

#include "macros.h"

#define INDEX_AND_RETURN(to_insert, which)                                     \
  {                                                                            \
    void *ptr = w_array_insert(&(physics_state.which), &to_insert);            \
    NULL_CHECK(ptr);                                                           \
    return ptr;                                                                \
  }
