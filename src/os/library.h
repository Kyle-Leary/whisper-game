#pragma once

// a cross-platform dll/so system.

#include "whisper/colmap.h"

// use a proc table for caching.
#define COMMON_FIELDS WColMap proc_table;

#ifdef __unix__
#define LIBRARY_PREFIX "lib"
#define LIBRARY_EXTENSION ".so"
typedef struct Lib {
  COMMON_FIELDS
  void *handle;
} Lib;
#else
#error "Unsupported platform"
#endif

// Lib* structures should be re-usable easily. this is what the api's designed
// around.

// just make the generic object, whatever's around the platform.
void make_lib(Lib *lib);
// actually load a library context into the object.
void load_lib(Lib *lib, const char *path);
void *lib_get_proc(Lib *lib, const char *name);
void lib_free(Lib *lib);
