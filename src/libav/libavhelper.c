#include "libavhelper.h"
#include "defines.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>

void sws_apply_context(struct SwsContext *sws_ctx, const int *input_stride,
                       int input_height, const int *dest_stride, byte *buffer,
                       byte *dest_buffer) {
  // scale into the same frame.
  sws_scale(sws_ctx, (const uint8_t *const *)buffer, input_stride, 0,
            input_height, (uint8_t *const *)dest_buffer, dest_stride);
}

AVCodecContext *av_codec_context_from_stream(AVStream *av_stream) {
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

void av_print_codec_ctx(AVCodecContext *codec_ctx) {
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

AVFormatContext *av_format_context_from_file(const char *file_path) {
  AVFormatContext *format_ctx = NULL;
  // get info about the file format, fill the format context.
  if (avformat_open_input(&format_ctx, file_path, NULL, NULL) < 0) {
    return NULL;
  }

  if (avformat_find_stream_info(format_ctx, NULL) < 0) {
    return NULL;
  }

  av_dump_format(format_ctx, 0, file_path, 0);

  return format_ctx;
}
