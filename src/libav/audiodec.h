#pragma once

#include "defines.h"
#include "whisper/queue.h"
#include <AL/al.h>
#include <bits/pthreadtypes.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#define MAX_AUDIO_BUFFER_FRAMES 10

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

void audio_fill_buffer(Audio *a, byte *dest);
void audio_fill_al_buffer(Audio *a, ALuint buffer);

// pass this an initted Audio*.
void *audio_decode_thread(void *data);
