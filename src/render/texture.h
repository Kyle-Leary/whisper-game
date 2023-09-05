#pragma once

#include "defines.h"
#include <sys/types.h>

#define NUM_TEXTURES 16

extern uint transparent_tex;
extern uint black_tex;
extern uint white_tex;

// a handle to a texture, an id can always be rep'd as a uint, likely.
typedef uint TextureHandle;

extern TextureHandle textures[NUM_TEXTURES];

typedef enum TextureType {
  // these enum variant values double as the slots that the textures are
  // inserted into.
  TEX_BASE_COLOR = 0,
  TEX_METALLIC_ROUGHNESS = 1,
  TEX_NORMAL = 2,
  TEX_OCCLUSION = 3,
  TEX_EMISSIVENESS = 4,
  TEX_COUNT,
} TextureType;

void g_use_pbr_texture(TextureType type, TextureHandle tex);

// returns an index into the global textures array, after loading it into the
// graphics backend.
uint g_load_texture(const char *filepath);
uint g_load_texture_from_png_buf(byte *png_buf, int len);
uint g_load_texture_from_buf(byte *img_buf, int width, int height,
                             int channels);

uint g_load_cubemap(char *faces[6]);
// "activates" the texture, using it for further draw calls.
// TODO: how to reason about texture slots?
void g_use_texture(TextureHandle handle, int slot);
void g_use_cubemap(TextureHandle handle, int slot);

void init_helper_textures();
