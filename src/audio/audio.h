#pragma once

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

// there's really not much in common here, other than the basic necessity of
// some lifetime functions. i won't lose sleep over it.

void a_init();
void a_update();
void a_clean();
void a_play_pcm(const char *filename); // play a wav file by path.
void a_kill_track(Track *t);
void a_kill_all();
