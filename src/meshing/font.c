#include <malloc.h>

#include "defines.h"
#include "font.h"
#include "global.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "backends/graphics_api.h"
#include "printers.h"
#include "util.h"

static u8 has_run = 0;

GraphicsRender *font_mesh_string(Font *font, const char *str, float x_char_size,
                                 float y_char_size) {
  int len = strlen(str);

  // get the whole vertex data for each character, all four points as seperate
  // vertices.
  int num_verts = len * 4;
  // Two tris for each character.
  int num_indices = len * 6;

  float positions[num_verts * 2];
  float uvs[num_verts * 2];

  uint indices[num_indices];

  // we start positioning the texture at the left edge, so that the positions
  // are nicely centered. changing the base position can better be accomplished
  // through the model matrix.
  float x_base = -(((float)len / 2) * x_char_size);
  float y_base = -(y_char_size / 2);

  float x_offset = 0;
  float y_offset = 0;

  // Populate vertices and UV arrays
  for (size_t i = 0; i < len; i++) {
    char target_ch = str[i];

    if (target_ch == '\n') {
      // TODO: handle newline properly.
      y_offset += y_char_size;
      x_offset = 0;
    }

    // we start the characters in the font at ' '.
    char target = target_ch - ' ';

    // we index into the uvs and positions arrays similarly, they have the
    // same layout.
    int base_idx = i * 8;

    {
      float x_min = x_base + x_offset;
      float y_min = y_base;
      // move over one tick from the min to find the max.
      float x_max = x_min + x_char_size;
      float y_max = y_min + y_char_size;

      // bottom left
      positions[base_idx + 0] = x_min;
      positions[base_idx + 1] = y_min;

      // bottom right
      positions[base_idx + 2] = x_max;
      positions[base_idx + 3] = y_min;

      // top right
      positions[base_idx + 4] = x_max;
      positions[base_idx + 5] = y_max;

      // top left
      positions[base_idx + 6] = x_min;
      positions[base_idx + 7] = y_max;
    }

    {
      // figure out the indices into the font image.
      int target_x = (target % font->RowStride);
      int target_y = font->ColStride - (target / font->RowStride) - 1;

      // bottom left
      float x_min = font->CharW * target_x;
      float y_min = font->CharH * target_y;
      // move over one tick from the min to find the max.
      float x_max = x_min + font->CharW;
      float y_max = y_min + font->CharW;

      uvs[base_idx + 0] = x_min;
      uvs[base_idx + 1] = y_min;

      // bottom right
      uvs[base_idx + 2] = x_max;
      uvs[base_idx + 3] = y_min;

      // top right
      uvs[base_idx + 4] = x_max;
      uvs[base_idx + 5] = y_max;

      // top left
      uvs[base_idx + 6] = x_min;
      uvs[base_idx + 7] = y_max;
    }

    { // generate indices (just basic quad stuff)
      int index_offset = i * 4;
      unsigned int charIndices[6] = {0 + index_offset, 1 + index_offset,
                                     2 + index_offset, 2 + index_offset,
                                     3 + index_offset, 0 + index_offset};
      memcpy(indices + (i * 6), charIndices, 6 * sizeof(unsigned int));
    }

    x_offset += x_char_size;
  }

  // Pass positions, uvs, and indices to your rendering function
  // (You will need to modify g_new_render to accept these separate arrays)
  GraphicsRender *gr = g_new_render(
      (VertexData *)&(HUDVertexData){RC_HUD, num_verts, positions, uvs},
      indices, num_indices);

  gr->pc = PC_HUD_TEXT;
  return gr;
}

// generate an RC_BASIC GraphicsRender from a const char*, highly similar to the
// normal font meshing function.
GraphicsRender *font_mesh_string_3d(Font *font, const char *str,
                                    float x_char_size, float y_char_size) {
  int len = strlen(str);

  // get the whole vertex data for each character, all four points as seperate
  // vertices.
  int num_verts = len * 4;
  // Two tris for each character.
  int num_indices = len * 6;

  float positions[num_verts * 3];
  float normals[num_verts * 3];
  float uvs[num_verts * 2];

  uint indices[num_indices];

  // we start positioning the texture at the left edge, so that the positions
  // are nicely centered. changing the base position can better be accomplished
  // through the model matrix.
  float x_base = -(((float)len / 2) * x_char_size);
  float y_base = -(y_char_size / 2);

  float x_offset = 0;
  float y_offset = 0;

  // Populate vertices and UV arrays
  for (size_t i = 0; i < len; i++) {
    char target_ch = str[i];

    if (target_ch == '\n') {
      // TODO: handle newline properly.
      y_offset += y_char_size;
      x_offset = 0;
    }

    // we start the characters in the font at ' '.
    char target = target_ch - ' ';

    {
      int base_idx = i * 12;

      float x_min = x_base + x_offset;
      float y_min = y_base;
      // move over one tick from the min to find the max.
      float x_max = x_min + x_char_size;
      float y_max = y_min + y_char_size;

      // bottom left
      positions[base_idx + 0] = x_min;
      positions[base_idx + 1] = y_min;
      positions[base_idx + 2] = 0;

      // bottom right
      positions[base_idx + 3] = x_max;
      positions[base_idx + 4] = y_min;
      positions[base_idx + 5] = 0;

      // top right
      positions[base_idx + 6] = x_max;
      positions[base_idx + 7] = y_max;
      positions[base_idx + 8] = 0;

      // top left
      positions[base_idx + 9] = x_min;
      positions[base_idx + 10] = y_max;
      positions[base_idx + 11] = 0;
    }

    { // normals.
      int base_idx = i * 12;
      for (int j = 0; j < 4; j++) {
        int norm_base_idx = base_idx + (j * 3);
        normals[norm_base_idx + 0] = 1;
        normals[norm_base_idx + 1] = 0;
        normals[norm_base_idx + 2] = 0;
      }
    }

    {
      int base_idx = i * 8;

      // figure out the indices into the font image.
      int target_x = (target % font->RowStride);
      int target_y = font->ColStride - (target / font->RowStride) - 1;

      // bottom left
      float x_min = font->CharW * target_x;
      float y_min = font->CharH * target_y;
      // move over one tick from the min to find the max.
      float x_max = x_min + font->CharW;
      float y_max = y_min + font->CharW;

      uvs[base_idx + 0] = x_min;
      uvs[base_idx + 1] = y_min;

      // bottom right
      uvs[base_idx + 2] = x_max;
      uvs[base_idx + 3] = y_min;

      // top right
      uvs[base_idx + 4] = x_max;
      uvs[base_idx + 5] = y_max;

      // top left
      uvs[base_idx + 6] = x_min;
      uvs[base_idx + 7] = y_max;
    }

    { // generate indices (just basic quad stuff)
      int index_offset = i * 4;
      unsigned int charIndices[6] = {0 + index_offset, 1 + index_offset,
                                     2 + index_offset, 2 + index_offset,
                                     3 + index_offset, 0 + index_offset};
      memcpy(indices + (i * 6), charIndices, 6 * sizeof(unsigned int));
    }

    x_offset += x_char_size;
  }

  // Pass positions, uvs, and indices to your rendering function
  // (You will need to modify g_new_render to accept these separate arrays)
  GraphicsRender *gr =
      g_new_render((VertexData *)&(BasicVertexData){RC_BASIC, num_verts,
                                                    positions, normals, uvs},
                   indices, num_indices);

  gr->pc = PC_TEXT_3D;
  return gr;
}

Font *font_init(unsigned char RowStride, unsigned char ColStride,
                TextureHandle texture_handle) {
  Font *font = malloc(sizeof(Font));

  font->RowStride = RowStride;
  font->ColStride = ColStride;
  font->CharW = 1.0 / RowStride; // assume the texture is always in screen-space
                                 // 1x1 coordinates.
  font->CharH = 1.0 / ColStride;
  font->tex_handle = texture_handle;

  return font;
}
