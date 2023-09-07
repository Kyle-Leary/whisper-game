// a simple level with a model in it to verify that the bind pose is working as
// intended.

#include "cglm/types.h"
#include "fmv.h"
#include "math/mat.h"
#include "objects/camera.h"
#include "path.h"
#include "render/gr_prim.h"
#include "render/graphics_render.h"
#include "render/light.h"
#include "render/material.h"
#include "render/material_render.h"
#include "render/model.h"
#include "render/render.h"
#include "render/texture.h"
#include "shaders/shader_instances.h"

#include "ogl_includes.h"

static vec3 camera_focus;

static Video *video;
static RenderComp *video_render;

#define GR ((GraphicsRender *)video_render->data.gr)

static YUVTex tex;

static void setup_video_render(GraphicsRender *gr) {
  g_use_texture(tex.y_tex, 0);
  g_use_texture(tex.u_tex, 1);
  g_use_texture(tex.v_tex, 2);
}

void areas_video() {
  glm_vec3_zero(camera_focus);

  video = fmv_init_video(VIDEO_PATH("my_friends.mp4"));

  video_render =
      make_rendercomp_from_graphicsrender(gr_prim_upright_plane((vec3){0}));

  glm_mat4_identity(GR->model);
  GR->shader = get_shader("yuv");
  GR->setup_fn = setup_video_render;

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);
}

void areas_video_update() {
  if (tex.y_tex != 0) {
    glDeleteTextures(3, (uint *)&tex);
  }

  fmv_get_frame_as_yuv_tex(&tex, video);
}
