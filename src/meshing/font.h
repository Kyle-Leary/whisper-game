#ifndef FONT_H
#define FONT_H

// font is basically just a meshing api. it creates vertices and indices, and
// returns a generic Render structure for later rendering.

#include "backends/graphics_api.h"
#include "defines.h"

#define ASCI_SYMBOL_OFT 32
#define ASCI_TOTAL_CHAR 128 - ASCI_SYMBOL_OFT
#define INVALID_UV -1.0f

// this is implementation independent.
typedef struct {
  float TexW, TexH;
  float CharW, CharH;
  unsigned char RowStride, ColStride;
  unsigned int TexFmt;
  unsigned int Color;
  unsigned int TexId;
  float TexUV[ASCI_TOTAL_CHAR][4];
  void *TexAddr;
} Font;

// this doesn't need to be stateful. just expose methods for other stateful
// modules to depend on.
GraphicsRender *
font_mesh_string(Font *font, const char *str, float x,
                 float y); // mesh the string from the font pointer and the
                           // string passed to it, create a static renderable.
Font *FontInit(float TexW, float TexH, unsigned char RowStride,
               unsigned char ColStride, u32 Color);

#endif
