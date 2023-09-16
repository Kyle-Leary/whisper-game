#include "fmv.h"
#include "audio/audio.h"
#include "libav/libavhelper.h"

// makes a thread and starts decoding the video, communicates to the main thread
// through an MQ. one thread per channel, for audio and video.
FMV *new_fmv(const char *file_path) {
  FMV *fmv = calloc(1, sizeof(FMV));
  strncpy(fmv->file_path, file_path, FILEPATH_MAX);

  // a container of stream data of several different formats and types,
  // potentially both audio and video.
  AVFormatContext *format_ctx = av_format_context_from_file(file_path);

  int video_stream_idx = -1;
  int audio_stream_idx = -1;

#define TRY_BREAK()                                                            \
  if (video_stream_idx != -1 && audio_stream_idx != -1)                        \
    break;

  for (int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_idx = i;
      TRY_BREAK();
    } else if (format_ctx->streams[i]->codecpar->codec_type ==
               AVMEDIA_TYPE_AUDIO) {
      audio_stream_idx = i;
      TRY_BREAK();
    }
  }

#undef TRY_BREAK

  if (video_stream_idx == -1) {
    fprintf(stderr, "No video stream found.\n");
    return NULL;
  } else if (audio_stream_idx == -1) {
    fprintf(stderr, "No audio stream found.\n");
    return NULL;
  }

  AVFormatContext *audio_format_ctx = av_format_context_from_file(file_path);

  fmv->video = new_video_from_format_and_idx(format_ctx, video_stream_idx);
  fmv->audio =
      new_audio_from_format_and_idx(audio_format_ctx, audio_stream_idx);

  return fmv;
}

void _empty_fn_fmv(ALuint buffer, Track *t) {
  audio_fill_al_buffer(t->data, buffer);
}

void fmv_play(FMV *fmv) {
  // pass the callback the AUDIO pointer, not the whole fmv. it should be able
  // to reason about the audio on its own.
  Track *t = a_new_stream(_empty_fn_fmv, fmv->audio);
  a_play_track(t);
}

void fmv_update(FMV *fmv) { video_poll_frame(fmv->video); }

void fmv_clean(FMV *fmv) {}
