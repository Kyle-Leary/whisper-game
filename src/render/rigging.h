#pragma once

// a maximum of 128 bones. this should be enough for our simpler animations.
#include "cglm/types.h"
#define BONE_LIMIT 128

// the data that will be sent directly to the GPU.
typedef struct BoneData {
  mat4 bones[BONE_LIMIT];
  // similarly, fill up the bones contiguously and dictate the maximum to the
  // gpu.
  int num_bones;
  char padding[12];
} BoneData;

// a skin has "joints" which is a list of Node indices in the global glb node
// array.
typedef struct Skin {
  // all the inverse bind matrices for every single bone in the armature
  // referencing this Skin.
  mat4 *ibms;
  int *joints;
  // one joint index and ibm for each joint in the skin.
  int num_joints;
} Skin;

void rig_use_bones(BoneData *bones);
