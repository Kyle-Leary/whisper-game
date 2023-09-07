#include "fmv.h"

#include "defines.h"
#include "global.h"
#include "helper_math.h"
#include "os.h"
#include "path.h"
#include "render/texture.h"
#include "whisper/macros.h"
#include "whisper/queue.h"
#include <bits/pthreadtypes.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libavutil/pixfmt.h>
#include <libavutil/rational.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct YUVFrame {
  int strides[3]; // y, u, v strides.
  byte *buffer;
} YUVFrame;

typedef struct AudioFrame {
  byte *buffer;
  int buffer_len;
} AudioFrame;

void _sws_apply_context(struct SwsContext *sws_ctx, const int *input_stride,
                        int input_height, const int *dest_stride, byte *buffer,
                        byte *dest_buffer) {
  // scale into the same frame.
  sws_scale(sws_ctx, (const uint8_t *const *)buffer, input_stride, 0,
            input_height, (uint8_t *const *)dest_buffer, dest_stride);
}

void _save_as_ppm(AVCodecContext *codec_ctx, AVFrame *frame, int frame_num) {
  // we now have the new frame.
  char ppm_file[256];
  sprintf(ppm_file, "frame%d.ppm", frame_num);
  FILE *f = fopen(ppm_file, "wb");

  fprintf(f, "P6\n%d %d\n255\n", codec_ctx->width, codec_ctx->height);

  for (int y = 0; y < codec_ctx->height; y++) {
    fwrite(frame->data[0] + y * frame->linesize[0], 1, codec_ctx->width * 3, f);
  }

  fclose(f);
}

AVCodecContext *_codec_context_from_stream(AVStream *av_stream) {
  AVCodecParameters *codecpar = av_stream->codecpar;
  const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);

  if (!codec) {
    fprintf(stderr, "Could not find codec.\n");
    return NULL;
  }

  AVCodecContext *codec_context = avcodec_alloc_context3(codec);

  if (!codec_context) {
    fprintf(stderr, "Could not allocate codec context.\n");
    return NULL;
  }

  if (avcodec_parameters_to_context(codec_context, codecpar) < 0) {
    fprintf(stderr, "Could not copy codec parameters to context.\n");
    avcodec_free_context(&codec_context);
    return NULL;
  }

  if (avcodec_open2(codec_context, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec.\n");
    avcodec_free_context(&codec_context);
    return NULL;
  }

  return codec_context;
}

void _print_codec_ctx(AVCodecContext *codec_ctx) {
  // Print codec name
  const AVCodec *codec = avcodec_find_decoder(codec_ctx->codec_id);
  if (codec != NULL) {
    printf("Codec: %s\n", codec->long_name);
  } else {
    printf("Codec not found!\n");
  }

  // Print pixel format
  const char *pix_fmt_name = av_get_pix_fmt_name(codec_ctx->pix_fmt);
  if (pix_fmt_name != NULL) {
    printf("Pixel Format: %s\n", pix_fmt_name);
  } else {
    printf("Unknown Pixel Format\n");
  }
}

void *audio_decode_thread(void *data) {
  Audio *a = (Audio *)data;
  AVCodecContext *codec_ctx = a->codec_ctx;
  AVFormatContext *format_ctx = a->format_ctx;
  int audio_stream_idx = a->audio_stream_idx;

  AVPacket packet;

  AVFrame *frame = av_frame_alloc();

  int sample_rate = codec_ctx->sample_rate;
  int nb_channels = codec_ctx->ch_layout.nb_channels;
  int bps = av_get_bytes_per_sample(codec_ctx->sample_fmt);

  double duration_in_seconds = format_ctx->streams[audio_stream_idx]->duration *
                               av_q2d(codec_ctx->time_base);

  uint frame_sz = (uint)(bps * nb_channels * sample_rate);

  // allocate more than enough space for the audio buffer, a full second of
  // audio (in this format/context).
  w_make_queue(&a->frame_queue, sizeof(AudioFrame), MAX_AUDIO_BUFFER_FRAMES);

  while (av_read_frame(format_ctx, &packet) >= 0) {
    if (packet.stream_index == audio_stream_idx) {
      // modern libav uses this audio decoding API where you send a packet to
      // the codec and recieve an audio frame directly, without much hassle.
      if (avcodec_send_packet(codec_ctx, &packet) < 0) {
        fprintf(stderr, "Error sending packet to decoder.\n");
        continue;
      }

      // pop through all the frames that the codec gets for us from the packet
      // we sent.
      while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
        // now we have the audio frame.
        // don't need the packet anymore.
        av_packet_unref(&packet);

        pthread_mutex_lock(&a->mutex);
        // assume that it's a one channel interleaved format.
        AudioFrame *audio_frame =
            (AudioFrame *)w_enqueue_alloc(&a->frame_queue);
        audio_frame->buffer = malloc(frame->nb_samples * bps);
        audio_frame->buffer_len = frame->nb_samples * bps;
        memcpy(audio_frame->buffer, frame->data[0], frame->nb_samples * bps);
        pthread_mutex_unlock(&a->mutex);

        for (;;) {
          pthread_mutex_lock(&a->mutex);
          int should_continue =
              (a->frame_queue.active_elements < MAX_AUDIO_BUFFER_FRAMES);
          pthread_mutex_unlock(&a->mutex);

          if (should_continue)
            break;

          musleep(10);
        }
      }

    } else {
      av_packet_unref(&packet);
    }
  }

  return NULL;
}

void *video_decode_thread(void *data) {
  Video *v = (Video *)data;
  AVCodecContext *codec_ctx = v->codec_ctx;
  AVFormatContext *format_ctx = v->format_ctx;
  int video_stream_idx = v->video_stream_idx;

  AVRational fr_rat = format_ctx->streams[video_stream_idx]->avg_frame_rate;
  float framerate = ((float)fr_rat.num) / fr_rat.den;
  v->framerate = framerate;

  AVFrame *frame = av_frame_alloc();
  AVPacket packet;

  v->w = codec_ctx->width;
  v->h = codec_ctx->height;

  int has_initted = 0;

  w_make_queue(&v->frame_queue, sizeof(YUVFrame), MAX_VIDEO_BUFFER_FRAMES);

  while (av_read_frame(format_ctx, &packet) >= 0) {
    if (packet.stream_index == video_stream_idx) {
      if (avcodec_send_packet(codec_ctx, &packet) >= 0) {
        while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
          // Now, frame->data contains the YUV data.

          int y_sz = frame->linesize[0] * v->h;
          int cb_sz = (frame->linesize[1] * v->h) / 2;
          int cr_sz = (frame->linesize[2] * v->h) / 2;
          int full_buf_sz = y_sz + cb_sz + cr_sz;

          pthread_mutex_lock(&v->mutex);
          YUVFrame *f = (YUVFrame *)w_enqueue_alloc(&v->frame_queue);
          f->buffer = calloc(full_buf_sz, 1);
          memcpy(f->buffer, frame->data[0], y_sz);
          memcpy(f->buffer + y_sz, frame->data[1], cb_sz);
          memcpy(f->buffer + y_sz + cb_sz, frame->data[2], cr_sz);
          memcpy(f->strides, frame->linesize, sizeof(int) * 3);
          pthread_mutex_unlock(&v->mutex);

          for (;;) {
            pthread_mutex_lock(&v->mutex);
            int should_continue =
                (v->frame_queue.active_elements < MAX_VIDEO_BUFFER_FRAMES);
            pthread_mutex_unlock(&v->mutex);

            if (should_continue)
              break;

            musleep(10);
          }
        }
      }
      av_packet_unref(&packet);
    } else {
      av_packet_unref(&packet);
    }
  }

  return NULL;
}

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
  v->format_ctx = format_ctx;
  v->video_stream_idx = video_stream_idx;
  v->codec_ctx =
      _codec_context_from_stream(format_ctx->streams[video_stream_idx]);
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
  a->format_ctx = audio_format_ctx;
  a->audio_stream_idx = audio_stream_idx;
  a->codec_ctx =
      _codec_context_from_stream(audio_format_ctx->streams[audio_stream_idx]);
  pthread_mutex_init(&a->mutex, NULL);
  pthread_create(&a->thread, NULL, audio_decode_thread, (void *)a);

  fmv->video = v;
  fmv->audio = a;

  return fmv;
}

void fmv_stop_video(Video *v) {
  pthread_join(v->thread, NULL);
  free(v);
}

void fmv_get_frame_as_yuv_tex(Video *v) {
  if (v->last_frame_time == 0) {
    // initializer case.
    v->last_frame_time = u_time;
  }

  v->frame_wait_time += u_time - v->last_frame_time;
  v->last_frame_time = u_time;

  if (v->frame_wait_time > (1 / v->framerate)) {
    v->frame_wait_time = 0;
  } else {
    // implicitly make the caller use the old frames, since they're still stuck
    // on the Video* structure.
    return;
  }

  pthread_mutex_lock(&v->mutex);
  if (v->frame_queue.active_elements == 0) {
    pthread_mutex_unlock(&v->mutex);
    return;
  }

  YUVFrame *yuv_frame = w_dequeue(&v->frame_queue);

  int y_sz = yuv_frame->strides[0] * v->h;
  int cb_sz = (yuv_frame->strides[1] * v->h) / 2;
  int cr_sz = (yuv_frame->strides[2] * v->h) / 2;
  int full_buf_sz = y_sz + cb_sz + cr_sz;

  // once we drop the lock, the frame will almost immediately be overwritten by
  // the other thread. we need to either copy it out of the queue or use it
  // right in the critical section.
  v->tex.y_tex = g_load_texture_from_buf(yuv_frame->buffer, v->w, v->h, 1,
                                         yuv_frame->strides[0]);
  v->tex.u_tex = g_load_texture_from_buf(yuv_frame->buffer + y_sz, v->w / 2,
                                         v->h / 2, 1, yuv_frame->strides[1]);
  v->tex.v_tex =
      g_load_texture_from_buf(yuv_frame->buffer + y_sz + cb_sz, v->w / 2,
                              v->h / 2, 1, yuv_frame->strides[2]);

  // then, clean up the frame, since we won't have access to it after the
  // unlock.
  free(yuv_frame->buffer);

  pthread_mutex_unlock(&v->mutex);
}

void audio_fill_buffer(Audio *a, byte *dest) {}

void fmv_init() {}

void fmv_clean() {}
