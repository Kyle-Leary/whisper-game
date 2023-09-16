#pragma once

#include "defines.h"
#include "whisper/queue.h"
#include "window.h"
#include <bits/pthreadtypes.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>

#define VR_QUEUE_SIZE 10

typedef struct VideoRecorder {
  pthread_t thread;
  pthread_mutex_t mutex;
  WQueue frame_queue;
  AVCodecContext *codec_ctx;
  AVFormatContext *format_ctx;
  char target_path[FILE_PATH_BUF_SZ];

  const int w, h; // the width and height of the video you're recording, this
                  // must stay constant over the course of the video encoding.

  int done_recording;
} VideoRecorder;

VideoRecorder *new_video_recorder(const char *target_path, int w, int h);

// for now, hardcode this to take in rgba planar data.
// the w and h of the screen/area might change, and the recorder should handle
// that internally by scaling the video? or maybe just crop the video?
void video_recorder_record_frame(VideoRecorder *vr, byte *frame, int w, int h);

void video_recorder_finish(VideoRecorder *vr);
