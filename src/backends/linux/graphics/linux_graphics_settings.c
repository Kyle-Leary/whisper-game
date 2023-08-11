#include "../ogl_includes.h"
#include "backends/graphics_api.h"
#include <stdio.h>

void g_set_depth_mode(DepthMode dm) {
  // simple wrapper setting in this case.
  switch (dm) {
  case DM_ON: {
    glEnable(GL_DEPTH_TEST);
  } break;
  case DM_OFF: {
    glDisable(GL_DEPTH_TEST);
  } break;
  default: {
    fprintf(stderr, "Invalid depth mode in g_set_depth_mode.\n");
  } break;
  }
}
