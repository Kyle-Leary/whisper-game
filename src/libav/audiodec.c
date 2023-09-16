#include "audiodec.h"
#include "audio/audio.h"
#include "defines.h"

#include "libav/libavhelper.h"
#include "os.h"

#include <AL/al.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <pthread.h>

typedef struct AudioFrame {
  byte *buffer;
  int buffer_len;
} AudioFrame;

void audio_fill_al_buffer(Audio *a, ALuint buffer) {
  pthread_mutex_lock(&a->mutex);

  int offset = 0;
  if (a->frame_queue.active_elements > 0) {
    AudioFrame *frame = w_dequeue(&a->frame_queue);

    byte *final_buffer = frame->buffer;
    int final_buffer_sz = frame->buffer_len;

    // openal expects interlaced pcm samples. libav/ffmpeg gives us either
    // planar or interlaced, so have this big switch to handle all the possible
    // weird edge cases with the audio conversion.

    ALenum al_fmt;
    if (a->nb_channels == 1) {
      switch (a->sample_fmt) {
      case AV_SAMPLE_FMT_U8: {
        al_fmt = AL_FORMAT_MONO8;
      } break;

      case AV_SAMPLE_FMT_S16: {
        al_fmt = AL_FORMAT_MONO16;
      } break;
      default: {
      } break;
      }
    } else {
      switch (a->sample_fmt) {
      case AV_SAMPLE_FMT_FLT: {
        al_fmt = AL_FORMAT_STEREO16;
      } break;

      case AV_SAMPLE_FMT_DBLP: {
      } break;

      case AV_SAMPLE_FMT_FLTP: {
        al_fmt = AL_FORMAT_STEREO16;

        // Number of float samples per channel
        int num_samples_per_channel =
            frame->buffer_len / (sizeof(float) * a->nb_channels);

        final_buffer_sz =
            num_samples_per_channel * sizeof(int16_t) * a->nb_channels;

        // Allocate a buffer for the 16-bit PCM data
        int16_t *pcm_buffer = malloc(final_buffer_sz);

        // Temporary pointer for readability
        float *fltp_buffer = (float *)frame->buffer;

        // Convert each channel from float to PCM and interleave
        for (int sample = 0; sample < num_samples_per_channel; ++sample) {
          for (int channel = 0; channel < a->nb_channels; ++channel) {
            float sample_value =
                fltp_buffer[num_samples_per_channel * channel + sample];
            int16_t to_copy = sample_value * 32767;
            pcm_buffer[sample * a->nb_channels + channel] = to_copy;
          }
        }

        final_buffer = (byte *)pcm_buffer;
      } break;

      default: {
        al_fmt = (a->nb_channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
      } break;
      }
    }

    alBufferData(buffer, al_fmt, final_buffer, final_buffer_sz, a->sample_rate);

    if (final_buffer != frame->buffer) {
      free(final_buffer);
    }
    free(frame->buffer);

    AL_ERR();
  }

  pthread_mutex_unlock(&a->mutex);
}

void audio_fill_buffer(Audio *a, byte *dest) {
  pthread_mutex_lock(&a->mutex);
  if (a->frame_queue.active_elements == 0) {
    pthread_mutex_unlock(&a->mutex);
    return;
  }

  AudioFrame *frame = w_dequeue(&a->frame_queue);
  memcpy(dest, frame->buffer, frame->buffer_len);
  pthread_mutex_unlock(&a->mutex);
}

void *audio_decode_thread(void *data) {
  os_thread_init();

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

  a->sample_rate = sample_rate;
  a->nb_channels = nb_channels;
  a->bps = bps;
  a->duration = duration_in_seconds;
  a->sample_fmt = codec_ctx->sample_fmt;

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
        a->has_started_reading = 1;

        // assume that it's a one channel interleaved format.
        AudioFrame *audio_frame =
            (AudioFrame *)w_enqueue_alloc(&a->frame_queue);
        audio_frame->buffer_len = frame->nb_samples * bps * nb_channels;
        audio_frame->buffer = malloc(audio_frame->buffer_len);

        // put the planar audio channels contiguously into the buffer.
        int offset = 0;
        for (int i = 0; i < a->nb_channels; i++) {
          memcpy(audio_frame->buffer + offset, frame->data[1],
                 frame->nb_samples * bps);
          offset += frame->nb_samples * bps;
        }

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

Audio *new_audio_from_format_and_idx(AVFormatContext *format_ctx, int idx) {
  for (int i = 0; i < format_ctx->nb_streams; ++i) {
    if (i != idx) {
      format_ctx->streams[i]->discard = AVDISCARD_ALL;
    }
  }

  Audio *a = calloc(1, sizeof(Audio));
  a->has_started_reading = 0;
  a->format_ctx = format_ctx;
  a->audio_stream_idx = idx;
  a->codec_ctx = av_codec_context_from_stream(format_ctx->streams[idx]);
  pthread_mutex_init(&a->mutex, NULL);
  pthread_create(&a->thread, NULL, audio_decode_thread, (void *)a);
  musleep(5); // very very ugly hack, we need to make sure that the audio thread
              // has some audio before we try to convert it into openal buffers,
              // so just wait.
  return a;
}

Audio *new_audio_from_format(AVFormatContext *format_ctx) {
  int audio_stream_idx = -1;

  for (int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audio_stream_idx = i;
      break;
    }
  }

  if (audio_stream_idx == -1) {
    fprintf(stderr, "No audio stream found.\n");
    return NULL;
  }

  return new_audio_from_format_and_idx(format_ctx, audio_stream_idx);
}

Audio *new_audio(const char *file_path) {
  // a container of stream data of several different formats and types,
  // potentially both audio and video.
  AVFormatContext *format_ctx = av_format_context_from_file(file_path);
  return new_audio_from_format(format_ctx);
}
