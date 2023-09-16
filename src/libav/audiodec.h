#pragma once

#include "defines.h"
#include "whisper/queue.h"
#include <AL/al.h>
#include <bits/pthreadtypes.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

// these will be used directly in the openal audio buffer queues.
#define MAX_AUDIO_BUFFER_FRAMES 100

// this structure represents a single stream of audio from a file. if you want
// multiple, specify multiple Audio* and streams using the different
// constructors.
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

Audio *new_audio_from_format_and_idx(
    AVFormatContext *format_ctx,
    int idx); // if you want to pick a specific track, use this function and
              // find the track yourself.
Audio *new_audio_from_format(AVFormatContext *format_ctx);
Audio *
new_audio(const char *file_path); // just grab the first audio stream we find in
                                  // the file and make an audio out of that.
