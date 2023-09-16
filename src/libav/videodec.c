#include "videodec.h"
#include "defines.h"
#include "global.h"
#include "libav/libavhelper.h"
#include "os.h"
#include "render/texture.h"
#include <libavutil/avutil.h>
#include <pthread.h>
#include <stdint.h>

typedef struct YUVFrame {
  int strides[3]; // y, u, v strides.
  byte *buffer;
} YUVFrame;

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

void *video_decode_thread(void *data) {
  os_thread_init();

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

  double pts = 0;

  while (av_read_frame(format_ctx, &packet) >= 0) {
    if (packet.stream_index == video_stream_idx) {
      if (avcodec_send_packet(codec_ctx, &packet) >= 0) {
        if (packet.dts != AV_NOPTS_VALUE) {
          pts = packet.dts;
          pts *= av_q2d(format_ctx->streams[video_stream_idx]->time_base);
        } else {
          pts = 0;
        }

        while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
          // Now, frame->data contains the YUV data.

          int y_sz = frame->linesize[0] * v->h;
          int cb_sz = (frame->linesize[1] * v->h) / 2;
          int cr_sz = (frame->linesize[2] * v->h) / 2;
          int full_buf_sz = y_sz + cb_sz + cr_sz;

          pthread_mutex_lock(&v->mutex);
          v->has_started_reading = 1;
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
    } else {
      av_packet_unref(&packet);
    }
  }

  return NULL;
}

void video_stop(Video *v) {
  pthread_join(v->thread, NULL);
  free(v);
}

void video_poll_frame(Video *v) {
  if (v->last_frame_time == 0) {
    // initializer case.
    v->last_frame_time = u_time;
  }

  v->frame_wait_time += u_time - v->last_frame_time;
  v->last_frame_time = u_time;

  float frame_time = 1 / v->framerate;
  if (v->frame_wait_time > frame_time) {
    v->frame_wait_time -= frame_time;
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

Video *new_video_from_format_and_idx(AVFormatContext *format_ctx, int idx) {
  for (int i = 0; i < format_ctx->nb_streams; ++i) {
    if (i != idx) {
      format_ctx->streams[i]->discard = AVDISCARD_ALL;
    }
  }

  Video *v = calloc(1, sizeof(Video));
  v->has_started_reading = 0;
  v->format_ctx = format_ctx;
  v->video_stream_idx = idx;
  v->codec_ctx = av_codec_context_from_stream(format_ctx->streams[idx]);
  pthread_mutex_init(&v->mutex, NULL);
  pthread_create(&v->thread, NULL, video_decode_thread, (void *)v);
  return v;
}

Video *new_video_from_format(AVFormatContext *format_ctx) {
  int video_stream_idx = -1;

  for (int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_idx = i;
      break;
    }
  }

  if (video_stream_idx == -1) {
    fprintf(stderr, "No video stream found.\n");
    return NULL;
  }

  return new_video_from_format_and_idx(format_ctx, video_stream_idx);
}

Video *new_video(const char *file_path) {
  // a container of stream data of several different formats and types,
  // potentially both video and video.
  AVFormatContext *format_ctx = av_format_context_from_file(file_path);
  return new_video_from_format(format_ctx);
}
