#include "audio.h"

#include "defines.h"
#include "path.h"

#include <AL/alut.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"

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

typedef struct AudioState {
  Track **tracks;
} AudioState;

#define MAX_TRACKS 6

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
  Track *t = (Track *)malloc(sizeof(Track));
  int filename_len = strlen(filename);
  t->filepath = (char *)malloc(filename_len);
  strncpy(t->filepath, filename, filename_len);

  // Create t->buffer and source
  alGenBuffers(1, &t->buffer);
  alGenSources(1, &t->source);

  // Load the wav file into the t->buffer
  t->buffer = alutCreateBufferFromFile(filename);

  // Set up source parameters
  alSourcei(t->source, AL_BUFFER, t->buffer); // al buffer contains source
  alSourcef(t->source, AL_PITCH, 1.0F);
  alSourcef(t->source, AL_GAIN, 0.1F);
  alSource3f(t->source, AL_POSITION, 0, 0, 0);
  alSource3f(t->source, AL_VELOCITY, 0, 0, 0);
  alSourcei(t->source, AL_LOOPING, AL_FALSE);

  for (int i = 0; i < MAX_TRACKS; i++) {
    if (a_s->tracks[i] == NULL) {
      // always break when the loop finishes. no unnecessary loops!!
      a_s->tracks[i] = t;
      break;
    }
  }

  return t;
}

static AudioState *
make_audio_state() { // should the state pre-malloc the track slots, or should
                     // the track initializer malloc itself? i think it's better
                     // to delay the initialization. it would be even better to
                     // static-alloc all of the slots, though. how do we do
                     // THAT? i don't really get the array[N] notation.
  AudioState *a = (AudioState *)calloc(sizeof(AudioState), 1);
  a->tracks = (Track **)calloc(sizeof(Track *), MAX_TRACKS);
  return a;
}

static void clean_audio_state(AudioState *a_s) {}

static AudioState *audio_state;

void a_play_pcm(const char *filename) {
  play_track(make_track(audio_state, filename));
}

void a_init() {
  audio_state = make_audio_state();
  alutInit((int *)calloc(1, 1), (char **)calloc(sizeof(char *), 0)); // lol

  // const char *filename = SOUND_PATH("smt1_home.wav");
  // Track *t = make_track(audio_state, filename);
  // play_track(t);
}

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

    alDeleteSources(1, &curr->source);
    alDeleteBuffers(1, &curr->buffer);

    free(curr);
    audio_state->tracks[i] = NULL;
  }
}

void a_clean() {
  clean_audio_state(audio_state);
  alutExit();
}
