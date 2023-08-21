#include "timescale.h"
#include "backends/input_api.h"
#include "global.h"
#include "object_bases.h"
#include "objects/label.h"
#include "render.h"

float fps = BASE_FPS;
float frame_time = BASE_FRAME_TIME;

static float time_scale = 1.0;

static void update_frame_time() { frame_time = 1.0 / fps; }

static Label *timescale_debug_render;
static char timescale_debug_str[512];
static const char *timescale_format_string = "timescale: %.02f, fps: %.02f";

void timescale_init() {
  sprintf(timescale_debug_str, timescale_format_string, time_scale, fps);
  timescale_debug_render = (Label *)object_add(
      (Object *)label_build(simple_font, (vec2){0.5, 0.9}, timescale_debug_str,
                            (vec3){0.3, 0.3, 0.3}),
      OT_PERMANENT);
}

void timescale_update() {
  if (i_state.act_just_pressed[ACT_INCREASE_TIMESCALE]) {
    timescale_scale_by(1.5);
  } else if (i_state.act_just_pressed[ACT_DECREASE_TIMESCALE]) {
    timescale_scale_by(0.5);
  }
}

void timescale_scale_by(float scale) {
  time_scale *= scale;
  fps = BASE_FPS * time_scale;
  update_frame_time();
  sprintf(timescale_debug_str, timescale_format_string, time_scale, fps);
  label_change_text(timescale_debug_render, timescale_debug_str);
}
