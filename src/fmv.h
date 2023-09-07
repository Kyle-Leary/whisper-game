#pragma once

#include "defines.h"
#include "whisper/queue.h"
#include <pthread.h>
#include <sys/types.h>

#define FILEPATH_MAX 256
#define MAX_BUFFER_FRAMES 10

typedef struct Video {
  pthread_t thread;
  pthread_mutex_t mutex;
  WQueue frame_queue;
  char file_path[FILEPATH_MAX];
  int w, h;
} Video;

typedef struct YUVTex {
  uint y_tex;
  uint u_tex;
  uint v_tex;
} YUVTex;

void fmv_init();
void fmv_clean();

Video *fmv_init_video(const char *file_path);
void fmv_stop_video(Video *v);
void fmv_get_frame_as_yuv_tex(YUVTex *dest, Video *v);
