#include "audio.h"

#include "defines.h"
#include "path.h"
#include "whisper/array.h"
#include <stdio.h>

static AudioState audio_state = {0};

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
  uint buf = alutCreateBufferFromFile(filename);
  if (buf == AL_NONE) {
    fprintf(stderr,
            "ERROR: Could not find '%s' while trying to make pcm buffer.\n",
            filename);
    return NULL;
  }

  t.buffer = buf;

  // Set up source parameters
  alSourcei(t.source, AL_BUFFER, t.buffer); // al buffer contains source
  alSourcef(t.source, AL_PITCH, 1.0F);
  alSourcef(t.source, AL_GAIN, 0.1F);
  alSource3f(t.source, AL_POSITION, 0, 0, 0);
  alSource3f(t.source, AL_VELOCITY, 0, 0, 0);
  alSourcei(t.source, AL_LOOPING, AL_FALSE);

  return w_array_insert(&audio_state.tracks, &t);
}

static void clean_audio_state(AudioState *a_s) {}

void a_play_pcm(const char *filename) {
  Track *t = make_track(&audio_state, filename);
  if (t != NULL) {
    play_track(t);
  } else {
    fprintf(stderr, "ERROR: Track is NULL, could not play pcm from '%s'.\n",
            filename);
  }
}

static int x = 0;

void a_init() {
  w_make_array(&audio_state.tracks, sizeof(Track), MAX_TRACKS);

  // pass dummy heap pointer.
  alutInit(&x, (char **)&x);

  const char *filename = SOUND_PATH("smt1_home.wav");
  Track *t = make_track(&audio_state, filename);
  play_track(t);
}

void a_free_track(Track *t) { free(t->filepath); }

void a_kill_track(Track *t) {
  alDeleteSources(1, &t->source);
  alDeleteBuffers(1, &t->buffer);

  a_free_track(t);
  w_array_delete_ptr(&audio_state.tracks, t);
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

    if (curr->state == AL_PLAYING)
      continue;

    // we've stopped playing, free the Track. looping is handled by openal, so
    // if it's really ended we don't have to worry about that.
    a_kill_track(curr);
  }
}

void a_clean() {
  clean_audio_state(&audio_state);
  alutExit();
}
