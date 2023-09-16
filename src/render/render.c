#include "render.h"
#include "meshing/gltf_mesher.h"
#include "parsers/gltf/gltf_parse.h"
#include "render/graphics_render.h"
#include "render/material_render.h"
#include "shaders/shader_instances.h"
#include "whisper/array.h"

static WArray render_comps;

RenderComp *make_rendercomp(RenderType type, void *data) {
  RenderComp rc;
  rc.type = type;
  rc.data.gr = data;
  rc.is_disabled = false;
  return w_array_insert(&render_comps, &rc);
}

RenderComp *make_rendercomp_from_glb(const char *path) {
  RenderComp rc;
  rc.type = RENDERTYPE_MODEL;
  rc.data.model = gltf_to_model(gltf_parse(path));
  rc.is_disabled = false;
  return w_array_insert(&render_comps, &rc);
}

RenderComp *make_rendercomp_from_graphicsrender(GraphicsRender *gr) {
  RenderComp rc;
  rc.type = RENDERTYPE_PRIMITIVE;
  rc.data.gr = gr;
  rc.is_disabled = false;
  return w_array_insert(&render_comps, &rc);
}

RenderComp *make_rendercomp_from_matrender(MaterialRender *mr) {
  RenderComp rc;
  rc.type = RENDERTYPE_MATERIAL;
  rc.data.mr = mr;
  rc.is_disabled = false;
  return w_array_insert(&render_comps, &rc);
}

void free_rendercomp(RenderComp *rc) {
  switch (rc->type) {
  case RENDERTYPE_MODEL: {
    free_model(rc->data.model);
  } break;
  case RENDERTYPE_PRIMITIVE: {
    free_graphics_render(rc->data.gr);
  } break;
  case RENDERTYPE_MATERIAL: {
    free_mat_render(rc->data.mr);
  } break;
  default: {
  } break;
  }
  w_array_delete_ptr(&render_comps, rc);
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
      g_draw_model(rc->data.model);
    } break;
    case RENDERTYPE_PRIMITIVE: {
      g_draw_render(rc->data.gr);
    } break;
    case RENDERTYPE_MATERIAL: {
      g_draw_mat_render(rc->data.mr);
    } break;
    default: {
    } break;
    }
  }
}

void render_clean() {}
