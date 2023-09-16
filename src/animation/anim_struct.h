#pragma once

typedef enum InterpolationType {
  IP_INVALID = 0,
  IP_LINEAR,
  IP_STEP, // no interpolation.
  IP_COUNT,
} InterpolationType;

// input, output and interpolation function.
typedef struct Sampler {
  // keyframe timing
  float *input; // always in seconds
  // keyframe output on the target.
  void *output; // some arbitrary datatype, depends on the target.

  // how we're going between keyframes in this animation sampler.
  InterpolationType interp;

  int num_frames;
} Sampler;

typedef struct Target {
  int node_index;
  char *property_name;
} Target;

// targets an index in the sampler array and a "target", a specific node and
// property in the glb file.
typedef struct Channel {
  Sampler *sampler;
  Target target;
  float channel_end_time;
} Channel;

typedef struct Animation {
  Sampler *samplers;
  int num_samplers;

  // playing back an animation is just playing through all the channels of the
  // animation at once on the target glb.
  Channel *channels;
  int num_channels;

  // optional name field, set to NULL if it doesn't exist?
  char *name;
} Animation;
