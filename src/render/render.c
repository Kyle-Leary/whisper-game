#include "render.h"
#include "meshing/gltf_mesher.h"
#include "parsers/gltf/gltf_parse.h"
#include "render/graphics_render.h"
#include "shaders/shader_instances.h"
#include "whisper/array.h"

static WArray render_comps;

RenderComp *make_rendercomp(RenderType type, void *data) {
  RenderComp rc;
  rc.type = type;
  rc.data = data;
  rc.is_disabled = false;
  return w_array_insert(&render_comps, &rc);
}

RenderComp *make_rendercomp_from_glb(const char *path) {
  RenderComp rc;
  rc.type = RENDERTYPE_MODEL;
  rc.data = gltf_to_model(gltf_parse(path));
  rc.is_disabled = false;
  return w_array_insert(&render_comps, &rc);
}

RenderComp *make_rendercomp_from_graphicsrender(GraphicsRender *gr) {
  RenderComp rc;
  rc.type = RENDERTYPE_PRIMITIVE;
  rc.data = gr;
  rc.is_disabled = false;
  return w_array_insert(&render_comps, &rc);
}

void render_init() {
  graphics_render_init();
  w_make_array(&(render_comps), sizeof(RenderComp), NUM_RENDER_COMPONENTS);
}

// logically update things internal to different renders.
void render_update() {}

// actually draw them.
void render_draw() {
  for (int i = 0; i < render_comps.upper_bound; i++) {
    RenderComp *rc = w_array_get(&render_comps, i);
    if (rc == NULL) {
      continue;
    }

    if (rc->is_disabled) {
      continue;
    }

    switch (rc->type) {
    case RENDERTYPE_MODEL: {
      g_draw_model(rc->data);
    } break;
    case RENDERTYPE_PRIMITIVE: {
      g_draw_render(rc->data);
    } break;
    default: {
    } break;
    }
  }
}

void render_clean() {}
