#include "mat.h"
#include "printers.h"

// in cglm/gl, matrices are stored column major, which is the opposite of how
// you might expect.
void mat4_mul(const mat4 left, const mat4 right, mat4 dest) {
  // dot the left's first row with the right's first column.
  //
  // dest[0][0] = ((left[0][0] * right[0][0]) + (left[1][0] * right[0][1]) +
  //               (left[2][0] * right[0][2]) + (left[3][0] * right[0][3]));
  mat4 temp;

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      temp[col][row] =
          ((left[0][row] * right[col][0]) + (left[1][row] * right[col][1]) +
           (left[2][row] * right[col][2]) + (left[3][row] * right[col][3]));
    }
  }

  memcpy(dest, temp, sizeof(float) * 16);
}

int mat4_equality(const mat4 a, const mat4 b) {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      if (a[col][row] != b[col][row])
        return 0;
    }
  }

  return 1;
}

void mat4_identity(mat4 dest) {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      if (row == col) {
        dest[col][row] = 1;
      } else {
        dest[col][row] = 0;
      }
    }
  }
}

void mat4_scale(const mat4 from, float scale, mat4 dest) {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      if (row == col) {
        dest[col][row] = from[col][row] * scale;
      } else {
        dest[col][row] = from[col][row];
      }
    }
  }
}
