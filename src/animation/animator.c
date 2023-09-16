#include "animator.h"
#include "animation/anim_struct.h"
#include "global.h"
#include "printers.h"
#include "properties.h"
#include "util.h"
#include "whisper/array.h"
#include "whisper/hashmap.h"

#include "macros.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

WArray animators = {0};

Animator *make_animator(Model *target) {
  Animator a;

  // we're using zero alloc as a sentinel on each of the anim entries.
  memset(a.anims, 0, sizeof(a.anims));
  // Animator animates a specific Model on each animation tick. maybe make this
  // more general, somehow?
  a.target = target;

  // again, just like the components return a global index into the array.
  return w_array_insert(&animators, &a);
}

static uint32_t murmur_hash3_32(const void *key, int len) {
  const uint8_t *data = (const uint8_t *)key;
  const int nblocks = len / 4;
  uint32_t h1 = 0; // hardcode the seed.
  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;
  const uint32_t *blocks = (const uint32_t *)(data + nblocks * 4);

  for (int i = -nblocks; i; i++) {
    uint32_t k1 = blocks[i];

    k1 *= c1;
    k1 = (k1 << 15) | (k1 >> 17); // ROTL32
    k1 *= c2;

    h1 ^= k1;
    h1 = (h1 << 13) | (h1 >> 19); // ROTL32
    h1 = h1 * 5 + 0xe6546b64;
  }

  const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
  uint32_t k1 = 0;

  switch (len & 3) {
  case 3:
    k1 ^= tail[2] << 16;
  case 2:
    k1 ^= tail[1] << 8;
  case 1:
    k1 ^= tail[0];
    k1 *= c1;
    k1 = (k1 << 15) | (k1 >> 17); // ROTL32
    k1 *= c2;
    h1 ^= k1;
  }

  // Finalization

  h1 ^= len;
  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  // the hash will never return zero.
  if (h1 == 0)
    h1++;

  return h1;
}

// pass it the top-level list of Nodes to be modified by the sampler.
// a and b are clamped timings inside the proper range.
static void tick_channel(Channel *c, Node *nodes, float a, float b) {
  Node *target_ref = &(nodes[c->target.node_index]);

  switch (IP_STEP) {
  case IP_STEP: {
    // implement generic STEP interpolation as a testing ground.
    int step_index = -1;
    for (int i = 0; i < c->sampler->num_frames; i++) {
      if (c->sampler->input[i] > b) {
        step_index = i;
        break;
      }
    }
    if (step_index == -1) // then default to the first index?
      step_index = 0;

    // get a ref to the beginning of the keyframe value data, and take that as
    // the step value base ptr. we're going to update the property on the target
    // with this value.
    int target_value_type_sz;
    int target_value_num_elms;
    void *target_value_ptr = return_prop_base_ptr(
        nodes, &c->target, &target_value_type_sz, &target_value_num_elms);
    void *keyframe_value =
        c->sampler->output +
        (target_value_type_sz * (step_index * (target_value_num_elms)));

    // right now, we're copying the data into the pointer each frame, even in a
    // STEP.
    memcpy(target_value_ptr, keyframe_value,
           target_value_type_sz * target_value_num_elms);
  } break;

  case IP_LINEAR: {
    int lower_index = -1;
    int upper_index = -1;
    for (int i = 0; i < c->sampler->num_frames - 1; i++) {
      if (c->sampler->input[i] <= b && c->sampler->input[i + 1] >= b) {
        lower_index = i;
        upper_index = i + 1;
        break;
      }
    }
    if (lower_index == -1) // Default to the first index
      lower_index = upper_index = 0;

    // Get pointers to the target and keyframe values
    int target_value_type_sz;
    int target_value_num_elms;
    float *target_value_ptr = (float *)return_prop_base_ptr(
        nodes, &c->target, &target_value_type_sz, &target_value_num_elms);
    float *keyframe_value_lower =
        (float *)&(c->sampler->output[(lower_index * target_value_num_elms)]);
    float *keyframe_value_upper =
        (float *)&(c->sampler->output[(upper_index * target_value_num_elms)]);

    // Calculate the interpolation factor
    float interp_factor = 0;
    if (c->sampler->input[lower_index] != c->sampler->input[upper_index]) {
      interp_factor =
          (b - c->sampler->input[lower_index]) /
          (c->sampler->input[upper_index] - c->sampler->input[lower_index]);
    }

    // Perform linear interpolation for each element
    for (int i = 0; i < target_value_num_elms; i++) {
      target_value_ptr[i] = (1.0f - interp_factor) * keyframe_value_lower[i] +
                            interp_factor * keyframe_value_upper[i];
    }
  } break;
  default: {
  } break;
  }
}

static void animation_update(Animator *animator, AnimEntry *entry) {
  // tick the channel from the time at the old frame to the time in the new
  // frame.
  float old_time = entry->curr_time;
  entry->curr_time += delta_time / 10;
  float new_time = entry->curr_time;

  Animation a = animator->target->animations[entry->model_anim_index];
  Sampler *samplers = a.samplers;
  Channel *channels = a.channels;
  for (int i = 0; i < a.num_channels; i++) {
    // an animation can be thought of as just iterating through channels.
    Channel c = channels[i];
    old_time = fmodf(old_time, c.channel_end_time);
    new_time = fmodf(new_time, c.channel_end_time);
    if (new_time < old_time) {
      // we've wrapped around.
      if (entry->loop) {
        // then this is fine, and we can just keep looping forever.
      } else {
        // remove self from the animation pool.
        entry->name_hash = 0;
        return; // if we're done with one channel, we're done with the whole
                // thing? maybe wait for the last channel to finish?
      }
    }

    tick_channel(&c, animator->target->nodes, old_time, new_time);
  }
}

// only update the animentries on the animator that are set to play.
static void animator_update(Animator *animator) {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    AnimEntry *entry = &(animator->anims[i]);
    // if it's a valid hash, then update the animation.
    if (entry->name_hash != 0 && entry->is_playing) {
      animation_update(animator, entry);
    }
  }
}

void anim_init() { w_make_array(&animators, sizeof(Animator), NUM_ANIMATORS); }

void anim_update() {
  for (int i = 0; i < animators.upper_bound; i++) {
    Animator *a = w_array_get(&animators, i);
    if (a) {
      animator_update(a);
    }
  }
}

void anim_clean() {}

void anim_force_tick(Animator *animator, int anim_entry_index) {
  if (anim_entry_index < 0 || anim_entry_index >= MAX_ANIMATIONS) {
    ERROR("Invalid animation entry index: '%d'\n", anim_entry_index);
    return;
  }

  AnimEntry *entry = &(animator->anims[anim_entry_index]);
  animation_update(animator, entry);
}

int anim_load(Animator *animator, const char *anim_name, bool should_loop) {
  AnimEntry ae;
  ae.model_anim_index = -1;
  // this isn't too bad. this data structure is optimized for access in the
  // update loop, since that's the most frequent use. the anim_play and _stop
  // methods are secondary to this module, and linear search is justified here.
  for (int i = 0; i < animator->target->num_animations; i++) {
    if (strcmp(animator->target->animations[i].name, anim_name) == 0) {
      // a match, found the animation index we're modifying with this AnimEntry.
      ae.model_anim_index = i;
      break;
    }
  }
  if (ae.model_anim_index == -1) {
    WARNING("Animation name \"%s\" not found.\n", anim_name);
    return -1;
  }

  Animation target_anim = animator->target->animations[ae.model_anim_index];

  ae.loop = should_loop;
  ae.curr_time = 0;
  ae.name_hash = murmur_hash3_32(anim_name, strlen(anim_name));

  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    AnimEntry *curr_ae = &(animator->anims[i]);
    if (!curr_ae->name_hash) { // if the name hash is zero, then it's invalid.
      // copy in the stack copy we've been working on throughout this function.
      memcpy(curr_ae, &ae, sizeof(AnimEntry));
      return i;
    }
  }

  ERROR("Too many animations on the Animator* %p. Exiting...\n", animator);
}

inline static void _animentry_set_playing(AnimEntry *ae) {
  ae->curr_time = 0;
  ae->is_playing = true;
}

void anim_play(Animator *animator, const char *anim_name, bool should_loop) {
  int i = anim_load(animator, anim_name, should_loop);
  if (i == -1) {
    ERROR("Failed to load animation '%s', could not play.", anim_name);
    return;
  }

  AnimEntry *ae = &(animator->anims[i]);
  _animentry_set_playing(ae);
}

static AnimEntry *_find_animentry(Animator *animator, const char *anim_name) {
  int hash = murmur_hash3_32(anim_name, strlen(anim_name));

  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    AnimEntry *curr_ae = &(animator->anims[i]);
    if (curr_ae->name_hash == hash) {
      return curr_ae;
    }
  }

  return NULL;
}

void anim_start(Animator *animator, const char *anim_name, bool should_loop) {
  AnimEntry *ae = _find_animentry(animator, anim_name);
  if (!ae) {
    // try to load the animation if it's not already found on the animator.
    INFO("anim_start: could not find anim '%s', loading it now.\n", anim_name);
    int i = anim_load(animator, anim_name, should_loop);
    if (i == -1) {
      // if we still can't find it, then we're done here.
      ERROR("Failed to load animation '%s', could not play.", anim_name);
      return;
    }
    ae = &(animator->anims[i]);
  }

  // we've found the animentry, start playing it.
  _animentry_set_playing(ae);
}

void anim_stop(Animator *animator, const char *anim_name) {
  uint32_t name_hash = murmur_hash3_32(anim_name, strlen(anim_name));

  AnimEntry *curr_ae = _find_animentry(animator, anim_name);
  if (!curr_ae) {
    WARNING("Could not find anim '%s', not stopping.\n", anim_name);
    return;
  }

  if (curr_ae->name_hash == name_hash) {
    curr_ae->is_playing = false;
    return;
  }
}

void anim_unload(Animator *animator, const char *anim_name) {
  uint32_t name_hash = murmur_hash3_32(anim_name, strlen(anim_name));

  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    AnimEntry *curr_ae = &(animator->anims[i]);
    if (curr_ae->name_hash ==
        name_hash) { // if the hashes match, chances are they're the same string
                     // and thus the same animation.
      curr_ae->name_hash =
          0; // zero out the name hash to signify an invalid value.
    }
  }
}
