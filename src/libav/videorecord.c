#include "videorecord.h"
#include "defines.h"
#include "os.h"
#include "timescale.h"
#include "whisper/queue.h"
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

static void *_video_record_thread(void *data) {
  os_thread_init();

  VideoRecorder *vr = (VideoRecorder *)data;

  enum AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;

  // Initialize format and format context
  AVFormatContext *fmt_ctx = NULL;
  int return_avformat_alloc =
      avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, vr->target_path);
  if (return_avformat_alloc < 0) {
    fprintf(stderr, "Could not allocate output context: %d\n",
            return_avformat_alloc);
    return NULL;
  }

  if (!fmt_ctx) {
    fprintf(stderr, "Could not allocate output context\n");
    return NULL;
  }

  if (!(fmt_ctx->flags & AVFMT_NOFILE)) {
    int avio_ret = avio_open(&fmt_ctx->pb, vr->target_path, AVIO_FLAG_WRITE);
    if (avio_ret < 0) {
      fprintf(stderr, "Could not open '%s': %s\n", vr->target_path,
              av_err2str(avio_ret));
      return NULL;
    }
  }

  // automatically use the best encoder for the container format the caller
  // selects.
  const AVCodec *codec = avcodec_find_encoder(fmt_ctx->oformat->video_codec);
  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    fprintf(stderr, "Could not allocate video codec context\n");
    return NULL;
  }

  codec_ctx->bit_rate = 400000;
  codec_ctx->gop_size = 12;
  codec_ctx->width = vr->w;
  codec_ctx->height = vr->h;
  codec_ctx->pix_fmt = pix_fmt;
  codec_ctx->time_base = (AVRational){1, (int)fps};

  /* open the codec */
  int open_ret = avcodec_open2(codec_ctx, codec, NULL);
  if (open_ret < 0) {
    fprintf(stderr, "Could not open video codec: %s\n", av_err2str(open_ret));
    exit(1);
  }

  // attach a video stream to the container mp4 format, make it have the
  // following const readonly codec.
  AVStream *video_stream = avformat_new_stream(fmt_ctx, codec);
  if (!video_stream) {
    fprintf(stderr, "Failed to create new stream\n");
    return NULL;
  }

  video_stream->time_base = codec_ctx->time_base;

  if (avcodec_parameters_from_context(video_stream->codecpar, codec_ctx) < 0) {
    fprintf(stderr, "Could not initialize stream codec parameters\n");
    return NULL;
  }

  // Write Header
  int header = avformat_write_header(fmt_ctx, NULL);
  if (header < 0) {
    fprintf(stderr, "Could not write header\n");
    return NULL;
  }

  // setup the generic params in the frame before we write the data into it, so
  // that the encoder knows what to do.
  AVFrame *frame = av_frame_alloc();
  {
    frame->width = vr->w;
    frame->height = vr->h;
    frame->format = pix_fmt;

    // allocate the buffers for the actual image data itself.
    if (av_frame_get_buffer(frame, 0) < 0) {
      fprintf(stderr, "Could not allocate frame data.\n");
      return NULL;
    }
  }

  AVFrame *temp_frame = av_frame_alloc();
  {
    temp_frame->width = vr->w;
    temp_frame->height = vr->h;
    temp_frame->format = pix_fmt;

    // allocate the buffers for the actual image data itself.
    if (av_frame_get_buffer(temp_frame, 0) < 0) {
      fprintf(stderr, "Could not allocate temp_frame data.\n");
      return NULL;
    }
  }
  // both frames are now allocated for both steps in the scaling process.

  AVPacket *pkt = av_packet_alloc();

  // everyone uses an int and not a uint?
  int64_t current_pts = 0;

  // use sws to scale from the caller's pix fmt to yuv420p colorspace.
  struct SwsContext *sws_color_tf_ctx;
  sws_color_tf_ctx =
      sws_getContext(vr->w, vr->h, AV_PIX_FMT_RGBA, vr->w, vr->h,
                     AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
  if (!sws_color_tf_ctx) {
    fprintf(stderr, "Could not initialize the conversion context\n");
    return NULL;
  }

  struct SwsContext *sws_flip_ctx;
  sws_flip_ctx =
      sws_getContext(vr->w, vr->h, AV_PIX_FMT_YUV420P, vr->w, vr->h,
                     AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

  // other threads mutate this variable to signal that it's done recording.
  while (!vr->done_recording) {
    if (avcodec_receive_packet(codec_ctx, pkt) == 0) {
      // if we have enough frames in the queue for the encoder to make a whole
      // packet, write it into the file with the format's specified output
      // context.
      av_interleaved_write_frame(fmt_ctx, pkt);
    }

    // then, decode the next frame from rgba to yuv420p from the queue.
    // loop until there's nothing left in the queue.
    pthread_mutex_lock(&vr->mutex);

    for (;;) {
      if (vr->frame_queue.active_elements == 0) {
        // if the queue is empty, stop encoding the frames from the queue.
        break;
      }

      // Dequeue the next frame
      uint8_t *buffer = (uint8_t *)w_dequeue(&vr->frame_queue);

      { // scale from src -> yuv -> flipped yuv.
        // Prepare a source data array and linesize
        uint8_t *src_data[1];
        int src_linesize[1];
        src_data[0] = buffer;
        src_linesize[0] = vr->w * 4; // 4 bytes for each pixel in RGBA

        // transform the yuv data into the temp frame.
        uint8_t *yuv_data[3];
        int yuv_linesize[3];
        yuv_data[0] = temp_frame->data[0]; // Y
        yuv_data[1] = temp_frame->data[1]; // U
        yuv_data[2] = temp_frame->data[2]; // V
        yuv_linesize[0] = temp_frame->linesize[0];
        yuv_linesize[1] = temp_frame->linesize[1];
        yuv_linesize[2] = temp_frame->linesize[2];

        // Perform the color space conversion
        sws_scale(sws_color_tf_ctx, (const uint8_t *const *)src_data,
                  src_linesize, 0, vr->h, yuv_data, yuv_linesize);

        // scale the actual, final frame into the destination frame.
        uint8_t *dst_data[3];
        int dst_linesize[3];
        dst_data[0] = frame->data[0];
        dst_data[1] = frame->data[1];
        dst_data[2] = frame->data[2];

        dst_linesize[0] = frame->linesize[0];
        dst_linesize[1] = frame->linesize[1];
        dst_linesize[2] = frame->linesize[2];

        // uint8_t *dst_data[3];
        // int dst_linesize[3];
        // dst_data[0] =
        //     temp_frame->data[0] + (temp_frame->linesize[0] * vr->h - 1);
        // dst_data[1] =
        //     temp_frame->data[1] + (temp_frame->linesize[1] * vr->h - 1);
        // dst_data[2] =
        //     temp_frame->data[2] + (temp_frame->linesize[2] * vr->h - 1);
        //
        // dst_linesize[0] = -temp_frame->linesize[0];
        // dst_linesize[1] = -temp_frame->linesize[1];
        // dst_linesize[2] = -temp_frame->linesize[2];

        // dst_data[0] = yuv_data[0] + (frame->linesize[0] * vr->h - 1);
        // dst_data[1] = yuv_data[1] + (frame->linesize[1] * vr->h - 1);
        // dst_data[2] = yuv_data[2] + (frame->linesize[2] * vr->h - 1);
        // dst_linesize[0] = -frame->linesize[0];
        // dst_linesize[1] = -frame->linesize[1];
        // dst_linesize[2] = -frame->linesize[2];

        // Perform the color space conversion
        sws_scale(sws_flip_ctx, (const uint8_t *const *)yuv_data, yuv_linesize,
                  0, vr->h, dst_data, dst_linesize);
      }

      // handle the frame's timestamps.
      frame->pts = current_pts++;
      // encoder handles the dts.

      // send the new frame to the encoder.
      if (avcodec_send_frame(codec_ctx, frame) < 0) {
        break;
      }
    }

    // prevent deadlock.
    pthread_mutex_unlock(&vr->mutex);
    musleep(10); // give the other thread some time to update.
  }

  av_packet_free(&pkt);

  // Write Trailer
  int trailer = av_write_trailer(fmt_ctx);
  if (trailer < 0) {
    fprintf(stderr, "Could not write trailer\n");
    return NULL;
  }

  // Clean up
  avcodec_close(codec_ctx);
  av_free(codec_ctx);
  av_free(frame);
  avio_close(fmt_ctx->pb);
  avformat_free_context(fmt_ctx);

  printf("Done recording video at path '%s'.\n", vr->target_path);

  return NULL;
}

VideoRecorder *new_video_recorder(const char *target_path, int w, int h) {
  VideoRecorder vr = {
      .w = w,
      .h = h,
  };

  vr.done_recording = 0;

  strncpy(vr.target_path, target_path, FILE_PATH_BUF_SZ);

  // take in raw rgba frames from glfw.
  w_make_queue(&vr.frame_queue, vr.w * vr.h * 4, VR_QUEUE_SIZE);

  pthread_mutex_init(&vr.mutex, NULL);

  VideoRecorder *recorder = malloc(sizeof(VideoRecorder));
  vr.thread = pthread_create(&vr.thread, NULL, _video_record_thread, recorder);
  memcpy(recorder, &vr, sizeof(VideoRecorder));

  printf("Started recording video at path '%s'.\n", vr.target_path);

  return recorder;
}

void video_recorder_record_frame(VideoRecorder *vr, byte *frame, int w, int h) {
  // for now, just throw an error.
  if (w != vr->w || h != vr->h) {
    fprintf(stderr, "ERROR - VideoRecorder: frame size mismatch.\n");
    return;
  }

  pthread_mutex_lock(&vr->mutex);
  w_enqueue(&vr->frame_queue, frame);
  pthread_mutex_unlock(&vr->mutex);
}

void video_recorder_finish(VideoRecorder *vr) { vr->done_recording = 1; }
