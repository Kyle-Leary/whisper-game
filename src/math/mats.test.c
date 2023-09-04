#include "mat.h"
#include "printers.h"

#include "macros.h"

void test_mats() {
  mat4 id;
  mat4_identity(id);

  {
    mat4 m;
    mat4_identity(m);
    RUNTIME_ASSERT(m[1][1] == 1.0);
    RUNTIME_ASSERT(m[0][1] == 0.0);
  }

  { // the identity by itself should remain the identity.
    mat4 dest;
    mat4_mul(id, id, dest);
    print_mat4(dest, 0);
    RUNTIME_ASSERT(mat4_equality(id, dest));
  }

  { // scale should only operate on the axes.
    mat4 dest;
    mat4_scale(id, 5, dest);
    RUNTIME_ASSERT(dest[1][1] == 5.0);
    RUNTIME_ASSERT(dest[0][1] == 0.0);
  }
}
