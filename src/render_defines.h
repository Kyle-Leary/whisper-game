#ifndef RENDER_DEFINES_H
#define RENDER_DEFINES_H

#include "cglm/types.h"
#include "defines.h"
#include <stdint.h>

typedef struct {
  vec3 position;
  u32 color;
} BasicVertex;

typedef struct BasicRender {
  BasicVertex *v;
  u16 *indices;
} BasicRender;

typedef struct { // use a gl-vertex layout oriented structure, rather than the
                 // normal ultra64.h Vtx union structure. this is actually the
                 // same layout as Vtx_n? probably not a coincidence?
  vec3 position;
  u32 color;
  float texcoord[2];
  float normal[3];
} FullVertex;

// really, we should always be using indices.
// glprim specifies basic rendering routines for all of these scenarios.
typedef struct FullRender {
  FullVertex *vertices; // list of vertices
  u16 *indices;         // list of indices
  u32 num_indices;      // number of indices in the list
} FullRender;

#endif // DEBUG
