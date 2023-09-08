// a simple level with a model in it to verify that the bind pose is working as
// intended.

#include "audio/audio.h"
#include "cglm/types.h"
#include "fmv.h"
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

static void empty_fn_cutscene(ALuint buffer) {
  int cond;
  while (1) {
    musleep(10);
    pthread_mutex_lock(&cutscene->audio->mutex);
    cond = cutscene->audio->has_started_reading;
    pthread_mutex_unlock(&cutscene->audio->mutex);
    if (cond)
      break;
  }

  audio_fill_al_buffer(cutscene->audio, buffer);
}

void areas_video() {
  glm_vec3_zero(camera_focus);

  cutscene = new_fmv(VIDEO_PATH("my_friends.mp4"));
  NULL_CHECK(cutscene);

  {
    Track *t = a_new_stream(empty_fn_cutscene);
    a_play_track(t);
  }

  // {
  //   Track *t = a_new_sine();
  //   a_play_track(t);
  // }
  // {
  //   Track *t = a_new_square();
  //   a_play_track(t);
  // }

  GraphicsRender *g = gr_prim_upright_plane((vec3){0});
  printf("g: %p, %d\n", g, g->n_idx);
  video_render = make_rendercomp_from_graphicsrender(g);

  glm_mat4_identity(GR->model);
  GR->shader = get_shader("yuv");
  GR->setup_fn = setup_video_render;

  Camera *cam = (Camera *)object_add(
      (Object *)camera_build((vec3){0}, &camera_focus), OT_AREA);
}

void areas_video_update() { fmv_get_frame_as_yuv_tex(cutscene->video); }
