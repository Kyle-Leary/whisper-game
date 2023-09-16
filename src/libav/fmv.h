#pragma once

// fmv is just a container structure for the audio and video decoded from a
// single file.

#include "defines.h"
#include "libav/audiodec.h"
#include "libav/videodec.h"
#include "whisper/queue.h"
#include <AL/al.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <pthread.h>
#include <sys/types.h>

#define FILEPATH_MAX 256

typedef struct FMV {
  Audio *audio;
  Video *video;
  char file_path[FILEPATH_MAX];
} FMV;

FMV *new_fmv(const char *file_path);

void fmv_play(FMV *fmv);
void fmv_update(FMV *fmv);
void fmv_clean(FMV *fmv);
