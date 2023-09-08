#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "whisper/array.h"

#include <AL/al.h>
#include <AL/alc.h>

// re-fill this buffer with data generically.
typedef void (*buffer_empty_fn)(ALuint buffer);

#define NUM_AUDIO_BUFFERS 10

typedef struct Track {
  ALuint buffers[NUM_AUDIO_BUFFERS]; // audio is double buffered by default. we
                                     // won't really be loading in sounds in
                                     // bulk at all.
  ALuint source;
  ALint state;

  // like usual, make this NULL if you're opting out.
  buffer_empty_fn empty_fn;
} Track;

#define MAX_TRACKS 6

typedef struct AudioState {
  WArray tracks;
} AudioState;

const char *al_enum_to_string(ALenum val);

void a_init();
void a_update();
void a_clean();

void a_play_track(Track *t);
void a_play_pcm(const char *filename); // play a wav file by path.
// stop playing the track.
void a_kill_track(Track *t);
void a_kill_all();

Track *a_new_stream(buffer_empty_fn empty_fn);
Track *a_new_file_track(const char *filename);

Track *a_new_sine();
Track *a_new_square();

#define AL_ERR()                                                               \
  {                                                                            \
    ALenum error = alGetError();                                               \
    if (error != AL_NO_ERROR) {                                                \
      fprintf(stderr, "OpenAL error: %s\n", al_enum_to_string(error));         \
    }                                                                          \
  }
