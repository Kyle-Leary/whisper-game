#include "transform.h"
#include "helper_math.h"
#include "whisper/macros.h"

void test_aabbtransform() {
  {
    AABB to = {0, 0, 1, 1};
    AABB by = {0.5, 0.5, 1, 1};
    AABB dest;

    aabb_apply_transform(&to, &by, &dest);

    ASSERT(by.center[0] == dest.center[0]);
  }
}
