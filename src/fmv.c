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
#include <libswscale/swscale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct YUVFrame {
  int strides[3]; // y, u, v strides.
  byte *buffer;
} YUVFrame;

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

void *fmv_decode_thread(void *data) {
  Video *v = (Video *)data;

  // a container of stream data of several different formats and types,
  // potentially both audio and video.
  AVFormatContext *format_ctx = NULL;

  // get info about the file format, fill the format context.
  if (avformat_open_input(&format_ctx, v->file_path, NULL, NULL) < 0) {
    return NULL;
  }

  if (avformat_find_stream_info(format_ctx, NULL) < 0) {
    return NULL;
  }

  av_dump_format(format_ctx, 0, v->file_path, 0);

  // populate format_ctx.streams.

  AVCodecContext *codec_ctx_init = NULL;
  AVCodecContext *codec_ctx = NULL;

  // search for a video stream.

  int vid_stream_idx = -1;
  for (int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      vid_stream_idx = i;
      break;
    }
  }

  if (vid_stream_idx == -1) {
    return NULL;
  }

  AVRational fr_rat = format_ctx->streams[vid_stream_idx]->avg_frame_rate;
  float framerate = ((float)fr_rat.num) / fr_rat.den;
  v->framerate = framerate;

  // the codec ctx used to be a param on the stream directly, now its a parallel
  // structure we have to derive from a codec ID manually.
  codec_ctx = _codec_context_from_stream(format_ctx->streams[vid_stream_idx]);

  _print_codec_ctx(codec_ctx);

  AVFrame *frame = av_frame_alloc();
  AVPacket packet;

  v->w = codec_ctx->width;
  v->h = codec_ctx->height;

  int has_initted = 0;

  w_make_queue(&v->frame_queue, sizeof(YUVFrame), MAX_BUFFER_FRAMES);

  while (av_read_frame(format_ctx, &packet) >= 0) {
    if (packet.stream_index == vid_stream_idx) {
      if (avcodec_send_packet(codec_ctx, &packet) >= 0) {
        while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
          // Now, frame->data contains the YUV data.
          // frame->data[0] contains Y plane
          // frame->data[1] contains U (Cb) plane
          // frame->data[2] contains V (Cr) plane

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
                (v->frame_queue.active_elements < MAX_BUFFER_FRAMES);
            pthread_mutex_unlock(&v->mutex);

            if (should_continue)
              break;

            musleep(10);
          }
        }
      }
    }

    av_packet_unref(&packet);
  }

  return NULL;
}

// makes a thread and starts decoding the video, communicates to the main thread
// through an MQ.
Video *fmv_init_video(const char *file_path) {
  // we need to manage the lifetime of this object, so it's just annoying to
  // make the caller stack allocate it.
  Video *v = calloc(1, sizeof(Video));
  pthread_mutex_init(&v->mutex, NULL);
  strncpy(v->file_path, file_path, FILEPATH_MAX);
  pthread_create(&v->thread, NULL, fmv_decode_thread, (void *)v);
  return v; // this Video* now has a thread running in the background tied to
            // decoding its frames.
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

void fmv_init() {}

void fmv_clean() {}
