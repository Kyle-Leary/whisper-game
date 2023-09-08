#pragma once

#include "whisper/queue.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <sys/types.h>

#define MAX_VIDEO_BUFFER_FRAMES 10

typedef struct YUVTex {
  uint y_tex;
  uint u_tex;
  uint v_tex;
} YUVTex;

typedef struct Video {
  pthread_t thread;
  pthread_mutex_t mutex;
  WQueue frame_queue;
  int w, h;
  float framerate;

  // video is yuv by default? do we need to support other decoding targets?
  YUVTex tex;

  float last_frame_time;
  float frame_wait_time; // helper counter to wait for the next frame when the
                         // client requests it.

  AVCodecContext *codec_ctx;
  AVFormatContext *format_ctx;
  int video_stream_idx;

  int has_started_reading;
} Video;

void video_stop(Video *v);
// maybe update the Video* yuv frame textures, and maybe not. this handles the
// framerate automatically based on the internal state of the Video*.
void video_poll_frame(Video *v);

// pass this an initted Video*.
void *video_decode_thread(void *data);
