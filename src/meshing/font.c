#include <malloc.h>

#include "defines.h"
#include "font.h"
#include "global.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/types.h>

#include "backends/graphics_api.h"

// we cache the letter positions on the atlas as we use them.
unsigned char FontUvNotCached(Font *font, int index) {
  return font->TexUV[index][0] == INVALID_UV;
}

static u8 has_run = 0;

GraphicsRender *font_mesh_string(Font *font, const char *str, float x,
                                 float y) {
  size_t len = strlen(str);

  float w, h;

  w = font->CharW;
  h = font->CharH;

  // get the whole vertex data for each character, all four points as seperate
  // vertices.
  uint num_verts = len * 4;
  // Two tris for each character.
  uint num_indices = len * 6;

  float positions[num_verts * 2]; // Position array
  float uvs[num_verts * 2];       // UV array

  uint indices[num_indices];

  // Populate vertices and UV arrays
  for (size_t i = 0; i < len; i++) {
    int index = str[i]; // Assuming ASCII
    int col = index / font->RowStride;
    int row = index % font->RowStride;

    if (FontUvNotCached(font, index)) {
      font->TexUV[index][0] = ((row * font->CharW)) / font->TexW;
      font->TexUV[index][1] = ((row * font->CharW) + font->CharW) / font->TexW;
      font->TexUV[index][2] = ((col * font->CharH)) / font->TexH;
      font->TexUV[index][3] = ((col * font->CharH) + font->CharH) / font->TexH;

#ifdef DEBUG
      printf("new TexUV[%d] in the cache: [%f, %f, %f, %f]\n", index,
             font->TexUV[index][0], font->TexUV[index][1],
             font->TexUV[index][2], font->TexUV[index][3]);
#endif /* ifdef DEBUG */
    }

    float xOffset = i * w; // Advance x position for each character

    // Populate position array for this character
    positions[i * 12] = x + xOffset;
    positions[i * 12 + 1] = y + h;
    positions[i * 12 + 2] = x + xOffset + w;
    positions[i * 12 + 3] = y + h;
    positions[i * 12 + 4] = x + xOffset + w;
    positions[i * 12 + 5] = y;
    positions[i * 12 + 6] = x + xOffset;
    positions[i * 12 + 7] = y;

    // Populate UV array for this character
    uvs[i * 8] = font->TexUV[index][0];
    uvs[i * 8 + 1] = font->TexUV[index][3];
    uvs[i * 8 + 2] = font->TexUV[index][1];
    uvs[i * 8 + 3] = font->TexUV[index][3];
    uvs[i * 8 + 4] = font->TexUV[index][1];
    uvs[i * 8 + 5] = font->TexUV[index][2];
    uvs[i * 8 + 6] = font->TexUV[index][0];
    uvs[i * 8 + 7] = font->TexUV[index][2];

    uint charIndices[6] = {0, 1, 2, 2, 3, 0};

    // copy the temp char buffers into the right places in the global function
    // state buffers that we'll eventually dump into a VBO and IBO all at once.
    memcpy(indices + i * 6, charIndices, 6 * sizeof(uint));
  }

  // Pass positions, uvs, and indices to your rendering function
  // (You will need to modify g_new_render to accept these separate arrays)
  return g_new_render(
      (VertexData *)&(HUDVertexData){RC_HUD, num_verts, positions, uvs},
      indices, num_indices);
}

Font *FontInit(float TexW, float TexH, unsigned char RowStride,
               unsigned char ColStride, u32 Color) {
  Font *font = malloc(sizeof(Font));

  font->TexW = TexW;
  font->TexH = TexH;
  font->RowStride = RowStride;
  font->ColStride = ColStride;
  font->CharW = (float)TexW / RowStride;
  font->CharH = (float)TexH / ColStride;
  font->Color = Color;

  unsigned short i;

  for (i = 0; i < ASCI_TOTAL_CHAR; i++)
    font->TexUV[i][0] = INVALID_UV;

  return font;
}
