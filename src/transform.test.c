#include "transform.h"
#include "helper_math.h"
#include "printers.h"
#include "whisper/macros.h"

void test_aabbtransform() {
  { // the identity transform should not do anything.
    AABB to = {0, 0, 1, 1};
    AABB by = {0.5, 0.5, 1, 1};
    AABB dest;

    aabb_apply_transform(&to, &by, &dest);

    ASSERT(to.center[0] == dest.center[0]);
    ASSERT(to.center[1] == dest.center[1]);
    ASSERT(to.extents[0] == dest.extents[0]);
    ASSERT(to.extents[1] == dest.extents[1]);
  }
}
