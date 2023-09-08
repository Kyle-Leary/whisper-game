#include "audio.h"

#include "defines.h"
#include "global.h"
#include "path.h"
#include "whisper/array.h"
#include "whisper/macros.h"
#include <AL/alc.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static AudioState audio_state = {0};

const char *al_enum_to_string(ALenum val) {
  switch (val) {
  case AL_NO_ERROR:
    return "AL_NO_ERROR";
  case AL_INVALID_NAME:
    return "AL_INVALID_NAME";
  case AL_INVALID_ENUM:
    return "AL_INVALID_ENUM";
  case AL_INVALID_VALUE:
    return "AL_INVALID_VALUE";
  case AL_INVALID_OPERATION:
    return "AL_INVALID_OPERATION";
  case AL_OUT_OF_MEMORY:
    return "AL_OUT_OF_MEMORY";
  case AL_INITIAL:
    return "AL_INITIAL";
  case AL_PLAYING:
    return "AL_PLAYING";
  case AL_PAUSED:
    return "AL_PAUSED";
  case AL_STOPPED:
    return "AL_STOPPED";
  default:
    return "UNKNOWN_ENUM";
  }
}

void a_play_track(Track *t) {
  // Play the sound
  alSourcePlay(t->source);
  alGetSourcei(t->source, AL_SOURCE_STATE,
               &t->state); // get the state as playing, and enter it into the
                           // polling loop in the audio_update.
}

static inline void _default_track_init(Track *t) {
  alGenBuffers(NUM_AUDIO_BUFFERS, t->buffers);
  alGenSources(1, &t->source);
}

static inline void _default_param_setup(Track *t) {
  // always play the first buffer, and use the second for buffering.
  alSourcef(t->source, AL_GAIN, 0.1F);
}

Track *a_new_stream(buffer_empty_fn empty_fn, void *data) {
  Track t;

  _default_track_init(&t);
  t.empty_fn = empty_fn;
  t.data = data;

#ifdef DEBUG
  if (alIsSource(t.source) != AL_TRUE) {
    fprintf(stderr, "ERROR: Source is invalid.\n");
  }
  for (int i = 0; i < NUM_AUDIO_BUFFERS; i++) {
    if (alIsBuffer(t.buffers[i]) != AL_TRUE) {
      fprintf(stderr, "ERROR: Buffer %d is invalid.\n", i);
    }
  }

  ALint source_state;
  alGetSourcei(t.source, AL_SOURCE_STATE, &source_state);
  if (source_state == AL_PLAYING) {
    fprintf(stderr, "ERROR: Source is already playing.\n");
  }
#endif

  // yes, init buffers BEFORE starting the queue, otherwise it'll error out on a
  // sourcequeuebuffers call.
  if (empty_fn) {
    for (int i = 0; i < NUM_AUDIO_BUFFERS; i++) {
      empty_fn(t.buffers[i], &t);
    }
  }

  // don't need to set the buffer param, since this overrides that.
  alSourceQueueBuffers(t.source, NUM_AUDIO_BUFFERS, t.buffers);

  _default_param_setup(&t);

  return w_array_insert(&audio_state.tracks, &t);
}

static void _sine_empty_fn(ALuint buffer, Track *t) {
  const float amp = 0.5;
  const int sample_rate = 44100;
  const float frequency = 440.0f; // Frequency in Hz
  const float dt = 1.0f / (float)sample_rate;
  int to_generate = 44100; // Number of samples to generate
  int16_t buf[to_generate];

  for (int i = 0; i < to_generate; ++i) {
    float t = u_time + i * dt;
    float sine_value = sinf(2.0f * M_PI * frequency * t) / amp;
    buf[i] = (int16_t)(sine_value * 32767);
  }

  alBufferData(buffer, AL_FORMAT_MONO16, buf, to_generate * sizeof(int16_t),
               sample_rate);
}

static void _square_empty_fn(ALuint buffer, Track *t) {
  const float amp = 0.5;
  const int sample_rate = 44100;
  const float frequency = 440.0f; // Frequency in Hz
  const float dt = 1.0f / (float)sample_rate;
  int to_generate = 44100; // Number of samples to generate
  int16_t buf[to_generate];

  for (int i = 0; i < to_generate; ++i) {
    float t = u_time + i * dt;
    float sine_value = sinf(2.0f * M_PI * frequency * t);
    int floored = floorf(sine_value + 0.5);
    buf[i] = floored * (32767 / amp);
  }

  alBufferData(buffer, AL_FORMAT_MONO16, buf, to_generate * sizeof(int16_t),
               sample_rate);
}

inline Track *a_new_sine() { return a_new_stream(_sine_empty_fn, NULL); }

inline Track *a_new_square() { return a_new_stream(_square_empty_fn, NULL); }

void a_play_pcm(const char *filename) {
  // init the Audio* with libav here.
  //
  // audio_fill_al_buffer(cutscene->audio, buffer);
}

static int x = 0;

static ALCdevice *alc_device;
static ALCcontext *alc_context;

void a_init() {
  w_make_array(&audio_state.tracks, sizeof(Track), MAX_TRACKS);

  alc_device = alcOpenDevice(NULL);
  if (!alc_device) {
    fprintf(stderr, "ERROR: Could not open audio device.\n");
    return;
  }
  alc_context = alcCreateContext(alc_device, NULL);
  if (!alc_context) {
    fprintf(stderr, "ERROR: Could not create audio context.\n");
    return;
  }

  if (!alcMakeContextCurrent(alc_context)) {
    fprintf(stderr, "ERROR: Could not make audio context current.\n");
    return;
  }

  AL_ERR();
}

void a_free_track(Track *t) {
  alDeleteSources(1, &t->source);
  alDeleteBuffers(2, t->buffers);

  // this will nullify the pointer in the array, and cause the track to be
  // skipped in the update. so, this does kill/mute the track.
  w_array_delete_ptr(&audio_state.tracks, t);
}

void a_kill_track(Track *t) {
  a_free_track(t);
  printf("Killed track %p.\n", t);
}

void a_kill_all() {
  for (int i = 0; i < audio_state.tracks.upper_bound; i++) {
    Track *curr = w_array_get(&audio_state.tracks, i);
    if (curr == NULL)
      continue;

    a_kill_track(curr);
  }
}

void a_update() {
  for (int i = 0; i < audio_state.tracks.upper_bound; i++) {
    Track *curr = w_array_get(&audio_state.tracks, i);

    if (curr == NULL)
      continue;

    alGetSourcei(curr->source, AL_SOURCE_STATE, &curr->state);

    if (curr->state == AL_PLAYING) {
      ALuint queue_buf;
      ALint nb_buffers;
      alGetSourcei(curr->source, AL_BUFFERS_PROCESSED, &nb_buffers);
      if (nb_buffers > 0) {
        if (curr->empty_fn) {
          // openal will mostly handle double buffering all on its own.
          alSourceUnqueueBuffers(curr->source, 1, &queue_buf);

          // now we can write data into queue_buf, put it back in the queue, and
          // we shouldn't have to worry about it anymore.
          curr->empty_fn(queue_buf, curr);

          alSourceQueueBuffers(curr->source, 1, &queue_buf);
          continue;
        } else {
          a_kill_track(curr);
        }
      }
      continue;
    } else {
      // we've stopped playing, free the Track. looping is handled by openal, so
      // if it's really ended we don't have to worry about that.
      a_kill_track(curr);
    }
  }
}

void a_clean() {
  alcMakeContextCurrent(NULL);
  alcDestroyContext(alc_context);
  alcCloseDevice(alc_device);
}
