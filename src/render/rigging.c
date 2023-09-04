#include "rigging.h"
#include "shaders/ubo.h"

void rig_use_bones(BoneData *bones) { ubo_push_bones(bones); }
