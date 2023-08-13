#include "backends/graphics_api.h"
#include <stdio.h>

PipelineConfiguration g_default_pc(RenderConfiguration conf) {
  switch (conf) {
  case RC_BASIC: {
    return PC_BLANK_GOURAUD; // provides nice ambient/light shading for simple
                             // textures bound to slot zero. a good default for
                             // pos/norm/uv models.
  } break;
  case RC_HUD: {
    return PC_HUD; // stuff like hud and text rendering.
  } break;
  case RC_MODEL: {
    return PC_MODEL; // proper rigging and bone data. a very very bulky
                     // pipeline.
  } break;
  default: {
    fprintf(stderr, "ERROR: You passed an invalid RenderConfiguration to "
                    "g_default_pc().\n");
    return PC_SOLID; // default to this i guess??
  } break;
  }
}
