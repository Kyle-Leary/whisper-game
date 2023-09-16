// a simple level with a model in it to verify that the bind pose is working as
// intended.

#include "audio/audio.h"
#include "cglm/types.h"
#include "global.h"
#include "gui/gui.h"
#include "gui/widgets.h"
#include "libav/fmv.h"
#include "libav/videodec.h"
#include "libav/videorecord.h"
#include "math/mat.h"
#include "objects/camera.h"
#include "os.h"
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

#include "macros.h"

#include "ogl_includes.h"
#include <AL/al.h>
#include <pthread.h>

static vec3 camera_focus;

static FMV *cutscene;
static RenderComp *video_render;

#define GR ((GraphicsRender *)video_render->data.gr)

static void setup_video_render(GraphicsRender *gr) {
  g_use_texture(cutscene->video->tex.y_tex, 0);
  g_use_texture(cutscene->video->tex.u_tex, 1);
  g_use_texture(cutscene->video->tex.v_tex, 2);
}

static VideoRecorder *vr = NULL;

void areas_video() {
  glm_vec3_zero(camera_focus);

  cutscene = new_fmv(VIDEO_PATH("my_friends.mp4"));
  NULL_CHECK(cutscene);
  fmv_play(cutscene);

  GraphicsRender *g = gr_prim_upright_plane((vec3){0});
  printf("g: %p, %d\n", g, g->n_idx);
  video_render = make_rendercomp_from_graphicsrender(g);

  glm_mat4_identity(GR->model);
  GR->shader = get_shader("yuv");
  GR->setup_fn = setup_video_render;

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);
}

void areas_video_update() {
  fmv_update(cutscene);

  gui_vert_push(0.1, 0.1);
  gui_widget("o");
  gui_widget("p");
  gui_widget("q");
  gui_widget("8");

  gui_horiz_push(0.02, 0.05);
  gui_widget("7");

  if (vr) {
    byte buf[window_get_frame_buffer_size()];
    window_get_frame_buffer(buf);
    video_recorder_record_frame(vr, buf, win_w, win_h);
  }

  if (gui_button("start", "start recording")) {
    if (vr)
      video_recorder_finish(vr);
    vr = new_video_recorder("hello.mp4", win_w, win_h);
  }

  if (gui_button("stop", "stop recording")) {
    if (vr)
      video_recorder_finish(vr);
    vr = NULL;
  }
}
