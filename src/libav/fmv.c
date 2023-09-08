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
  AVFormatContext *format_ctx = NULL;

  // get info about the file format, fill the format context.
  if (avformat_open_input(&format_ctx, fmv->file_path, NULL, NULL) < 0) {
    return NULL;
  }

  if (avformat_find_stream_info(format_ctx, NULL) < 0) {
    return NULL;
  }

  av_dump_format(format_ctx, 0, fmv->file_path, 0);

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

  // TODO: we can filter out streams that we don't care about on certain
  // contexts to save time and memory.
  //
  // for (int i = 0; i < format_ctx->nb_streams; ++i) {
  //   if (i != video_stream_idx) {
  //     format_ctx->streams[i]->discard = AVDISCARD_ALL;
  //   }
  // }

  // these should both be able to run in parallel and be used independently.
  Video *v = calloc(1, sizeof(Video));
  v->has_started_reading = 0;
  v->format_ctx = format_ctx;
  v->video_stream_idx = video_stream_idx;
  v->codec_ctx =
      av_codec_context_from_stream(format_ctx->streams[video_stream_idx]);
  pthread_mutex_init(&v->mutex, NULL);
  // pass the pointer to the substructure through the args, since we've attached
  // all the necessary file data to the struct.
  pthread_create(&v->thread, NULL, video_decode_thread, (void *)v);

  // format context reads aren't thread safe, so i guess just open a new one?
  // this is ugly.
  AVFormatContext *audio_format_ctx = NULL;
  // get info about the file format, fill the format context.
  if (avformat_open_input(&audio_format_ctx, fmv->file_path, NULL, NULL) < 0) {
    return NULL;
  }
  if (avformat_find_stream_info(audio_format_ctx, NULL) < 0) {
    return NULL;
  }

  Audio *a = calloc(1, sizeof(Audio));
  a->has_started_reading = 0;
  a->format_ctx = audio_format_ctx;
  a->audio_stream_idx = audio_stream_idx;
  a->codec_ctx =
      av_codec_context_from_stream(audio_format_ctx->streams[audio_stream_idx]);
  pthread_mutex_init(&a->mutex, NULL);
  pthread_create(&a->thread, NULL, audio_decode_thread, (void *)a);

  fmv->video = v;
  fmv->audio = a;

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
