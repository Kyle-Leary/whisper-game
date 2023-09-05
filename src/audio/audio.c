#include "audio.h"

#include "defines.h"
#include "path.h"

#include <AL/alut.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "whisper/array.h"

// compiling this with -lopenal and -lalut, two different dynlibs.
// openal
#include <AL/al.h>
#include <AL/alc.h>
// wrapper library around openal
#include <AL/alut.h>

typedef struct Track {
  ALuint buffer, source;
  ALint state;
  char *filepath; // path to the audio file it represents.
} Track;

#define MAX_TRACKS 6

typedef struct AudioState {
  WArray tracks;
} AudioState;

static AudioState *audio_state;

static void play_track(Track *t) {
  // Play the sound
  alSourcePlay(t->source);
  alGetSourcei(t->source, AL_SOURCE_STATE,
               &t->state); // get the state as playing, and enter it into the
                           // polling loop in the audio_update.
}

// create the track, and add it in a non-null spot in the tracklist
// in the passed AudioState structure.
static Track *make_track(AudioState *a_s, const char *filename) {
  Track t;

  int filename_len = strlen(filename);
  t.filepath = (char *)malloc(filename_len);
  strncpy(t.filepath, filename, filename_len);

  // Create t.buffer and source
  alGenBuffers(1, &t.buffer);
  alGenSources(1, &t.source);

  // Load the wav file into the t.buffer
  t.buffer = alutCreateBufferFromFile(filename);

  // Set up source parameters
  alSourcei(t.source, AL_BUFFER, t.buffer); // al buffer contains source
  alSourcef(t.source, AL_PITCH, 1.0F);
  alSourcef(t.source, AL_GAIN, 0.1F);
  alSource3f(t.source, AL_POSITION, 0, 0, 0);
  alSource3f(t.source, AL_VELOCITY, 0, 0, 0);
  alSourcei(t.source, AL_LOOPING, AL_FALSE);

  return w_array_insert(&audio_state->tracks, &t);
}

static void clean_audio_state(AudioState *a_s) {}

void a_play_pcm(const char *filename) {
  play_track(make_track(audio_state, filename));
}

static int x = 0;

void a_init() {
  // pass dummy heap pointer.
  alutInit(&x, (char **)&x);

  const char *filename = SOUND_PATH("smt1_home.wav");
  Track *t = make_track(audio_state, filename);
  play_track(t);
}

void a_free_track(Track *t) { free(t->filepath); }

void a_stop_track(Track *t) {
  alDeleteSources(1, &t->source);
  alDeleteBuffers(1, &t->buffer);

  a_free_track(t);
  w_array_delete_index(WArray * array, uint index)
}

void a_stop_all() {}

void a_update() {
  for (int i = 0; i < MAX_TRACKS; i++) {
    Track *curr = audio_state->tracks[i];

    if (curr == NULL)
      continue;

    // for example, we have a reference to each track and can modify them in a
    // loop.
    //
    // alSourcef(curr->source, AL_GAIN, vol);
    //
    // there's no binding, just pass in the source pointer and it works.

    alGetSourcei(curr->source, AL_SOURCE_STATE, &curr->state);

    if (curr->state == AL_PLAYING)
      continue;

    // we've stopped playing, free and reset the Track.
  }
}

void a_clean() {
  clean_audio_state(audio_state);
  alutExit();
}
