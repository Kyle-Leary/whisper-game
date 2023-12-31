#pragma once

#include "animation/anim_struct.h"
#include "render/model.h"
#include "whisper/array.h"
#include <stdbool.h>
#include <stdint.h>

// how many things can be currently animating?
#define NUM_ANIMATORS 16

// how many animations can be on a model at one time?
#define MAX_ANIMATIONS 16

typedef struct AnimEntry {
  int model_anim_index; // in the Animation* array on the model, which index is
                        // this AnimEntry describing the playback of?
  float curr_time;

  bool loop; // should the animation replay when it's finished?

  // this is weird. for the name_hash, 0 is the sentinel invalid value to tell
  // us that the AnimEntry is invalid. if this is zero, it won't be run through
  // in the update at all.
  uint32_t name_hash; // only store the hash of a name, it's all we need.

  bool is_playing;
} AnimEntry;

// animator structure targeting a single model. each movable model has an
// Animator that is ticked through and managed by the anim_ module.
typedef struct Animator {
  AnimEntry anims[MAX_ANIMATIONS];
  Model *target; // who are all these animations imposed on?
} Animator;

// other objects are managing their own animators, so this should be a list of
// pointers.
extern WArray animators;

Animator *make_animator(Model *target);

void anim_init();
void anim_update();
void anim_clean();

void anim_insert(Animator *animator);
// void anim_remove(Animator *animator);

// use the return value from anim_load to forcefully/programatically move one
// frame through the animation.
void anim_force_tick(Animator *animator, int anim_entry_index);

int anim_load(Animator *animator, const char *anim_name, bool should_loop);

// load then play. slightly more expensive than anim_start.
void anim_play(Animator *animator, const char *anim_name, bool should_loop);
// try to play, but don't load if it's not already loaded.
void anim_start(Animator *animator, const char *anim_name, bool should_loop);

void anim_stop(Animator *animator, const char *anim_name);

void anim_unload(Animator *animator, const char *anim_name);
