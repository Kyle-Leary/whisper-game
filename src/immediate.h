#pragma once

#include "cglm/types.h"
#include <sys/types.h>

#define MAX_IM_DRAWS 200

typedef enum IMDrawMode {
  IM_INVALID = 0,
  IM_TRIANGLES,
  IM_POLYGON,
  IM_LINE_STRIP,
  IM_LINES,
  IM_POINTS,
  IM_COUNT,
} IMDrawMode;

typedef struct IMDrawCall {
  // we can assume the hardcoded vertex size of 3 floats for now.
  uint num_vertices;
  uint vao_idx; // the index into the buffer's vao array.
  IMDrawMode mode;
  vec3 color;
} IMDrawCall;

typedef struct IMBuffer {
  // 0 is sentinel, that's the invalid value for all the opengl buffer types,
  // binding 0 unbinds.
  IMDrawCall draws[MAX_IM_DRAWS];
  uint vaos[MAX_IM_DRAWS]; // batch the vao deletion calls to opengl into ONE
                           // CALL by setting all of these up contiguously.
  uint num_draws;          // how many are currently in use this pass?
} IMBuffer;

void im_draw(float *vertices, uint num_vertices, vec3 color, IMDrawMode mode);

void im_init();

// flush out and actually draw all the stored shapes.
void im_flush();
