#pragma once

#include <sys/types.h>

// we actually do have a use for data other than float*'s, just make a void*.
uint make_vbo(const void *data, unsigned int count, unsigned int sizeof_vtx);
