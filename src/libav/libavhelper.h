#pragma once

#include "defines.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

void sws_apply_context(struct SwsContext *sws_ctx, const int *input_stride,
                       int input_height, const int *dest_stride, byte *buffer,
                       byte *dest_buffer);
AVCodecContext *av_codec_context_from_stream(AVStream *av_stream);
void av_print_codec_ctx(AVCodecContext *codec_ctx);
