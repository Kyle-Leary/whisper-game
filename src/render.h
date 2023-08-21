#pragma once

// this is a rendering subsystem that runs over RenderComp structures to draw
// everything in the game to the screen. this removes the need for a proper
// Object draw-> call and pushes more stuff into a different subsystem than the
// Object subsystem.

#include "backends/graphics_api.h"
#include <stdbool.h>

#define NUM_RENDER_COMPONENTS 200

typedef enum RenderType {
  RENDERTYPE_PRIMITIVE,
  RENDERTYPE_MODEL,
  RENDERTYPE_COUNT,
} RenderType;

typedef struct RenderComp {
  bool is_disabled; // stop rendering if this is toggled.

  RenderType type; // switch over several different types of renders, all
                   // handled here in the render subsystem.
  void *data;
} RenderComp;

// similar to the physics components, insert into the global render array and
// return the render component ptr.
RenderComp *make_rendercomp(RenderType type, void *data);
RenderComp *make_rendercomp_from_glb(const char *path);
RenderComp *make_rendercomp_from_graphicsrender(GraphicsRender *gr);

void render_init();
void render_update();
void render_draw();
void render_clean();
