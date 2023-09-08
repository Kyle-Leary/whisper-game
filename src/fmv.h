#pragma once

#include "defines.h"
#include "whisper/queue.h"
#include <AL/al.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <pthread.h>
#include <sys/types.h>

#define FILEPATH_MAX 256
#define MAX_VIDEO_BUFFER_FRAMES 10
#define MAX_AUDIO_BUFFER_FRAMES 10

typedef struct YUVTex {
  uint y_tex;
  uint u_tex;
  uint v_tex;
} YUVTex;

typedef struct Audio {
  pthread_t thread;
  pthread_mutex_t mutex;
  // audio chunks can also be called frames.
  WQueue frame_queue;

  AVCodecContext *codec_ctx;
  AVFormatContext *format_ctx;
  enum AVSampleFormat sample_fmt;
  int audio_stream_idx;
  int nb_channels;
  int sample_rate;
  int bps;
  double duration;

  int has_started_reading;
} Audio;

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

typedef struct FMV {
  Audio *audio;
  Video *video;
  char file_path[FILEPATH_MAX];
} FMV;

void fmv_init();
void fmv_clean();

void audio_fill_buffer(Audio *a, byte *dest);

FMV *new_fmv(const char *file_path);
void fmv_stop_video(Video *v);
void fmv_get_frame_as_yuv_tex(Video *v);
void audio_fill_al_buffer(Audio *a, ALuint buffer);
