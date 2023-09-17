#include "path.h"
#include "os/library.h"
#include <stdio.h>

void area_lib_path(char *out, const char *area_name) {
  sprintf(out, AREA_PATH LIBRARY_PREFIX "area%s" LIBRARY_EXTENSION, area_name);
}
