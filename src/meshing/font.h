#ifndef FONT_H
#define FONT_H

// font is basically just a meshing api. it creates vertices and indices, and
// returns a generic Render structure for later rendering.

#include "backends/graphics_api.h"
#include "defines.h"

// this is implementation independent.
typedef struct {
  float CharW, CharH;
  unsigned char RowStride, ColStride;
  TextureHandle tex_handle;
} Font;

// expose a simpler method for those not using the full GraphicsRender api.
void font_mesh_string_raw(Font *font, const char *str, uint strlen,
                          float x_char_size, float y_char_size,
                          float *positions, float *uvs, uint *indices);

// pass the size you want each character to be.
// this function will return a render containing centered text geometry.
// for example, if you render this at the screen offset 0.5 0.5, it'll be
// perfectly centered in the middle of the screen.
GraphicsRender *font_mesh_string(Font *font, const char *str, float x_char_size,
                                 float y_char_size);
GraphicsRender *font_mesh_string_3d(Font *font, const char *str,
                                    float x_char_size, float y_char_size);

Font *font_init(unsigned char RowStride, unsigned char ColStride,
                TextureHandle texture_handle);

#endif
