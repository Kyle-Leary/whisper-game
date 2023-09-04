#pragma once

#include "defines.h"
#include "render/model.h"
#include "wjson.h"
#include <wchar.h>

// types for the accessor buffers.
typedef enum {
  GLTF_BYTE = 5120,           // signed 8-bit integer
  GLTF_UNSIGNED_BYTE = 5121,  // unsigned 8-bit integer
  GLTF_SHORT = 5122,          // signed 16-bit integer
  GLTF_UNSIGNED_SHORT = 5123, // unsigned 16-bit integer
  GLTF_UNSIGNED_INT = 5125,   // unsigned 32-bit integer
  GLTF_FLOAT = 5126           // 32-bit floating point
} GLTF_ComponentType;

typedef struct GLTFFile {
  // header and section header defined stuff
  u32 version;
  u32 gltf_length;

  // json and binary gltf chunks can be in either order.

  u32 binary_offset;
  u32 binary_length;
  u8 *binary_data;

  u32 *buffer_offsets; // for easy access, store a list of offsets directly in
                       // the GLTFFile. mostly, a buffer field is only good for
                       // its offset data, so avoid grabbing it multiple times
                       // and just do it here.
  u32 num_buffers;

  u32 json_offset;
  u32 json_length;

  // then, helper fields that pull apart the JSON structure of the GLTF JSON
  // chunk a bit.
  WJSONValue *asset;
  WJSONValue *scenes;
  WJSONValue *nodes;
  WJSONValue *meshes;
  WJSONValue *materials;
  WJSONValue *textures;
  WJSONValue *images;
  WJSONValue *samplers;
  WJSONValue *accessors;
  WJSONValue *bufferViews;
  WJSONValue *buffers;
  WJSONValue *skins;
  WJSONValue *animations;
  WJSONValue *cameras;
  WJSONValue *extensions;
  WJSONValue *extras;
} GLTFFile;

GLTFFile *gltf_parse(const char *file_path);

void gltf_file_free(GLTFFile *file);

int gltf_bv_get_len(GLTFFile *file, int index);
void gltf_bv_parse(GLTFFile *file, int index, void *dest);

// all of these functions additonally return the size of the
// buffer created.
#define DEFINE_DUMP_TYPE_HEADER(name, type_name)                               \
  int gltf_dump_##name##_accessor(GLTFFile *file, int accessor_idx,            \
                                  type_name *buffer, int *n_elms);

DEFINE_DUMP_TYPE_HEADER(float, float)
DEFINE_DUMP_TYPE_HEADER(ushort, unsigned short)
DEFINE_DUMP_TYPE_HEADER(ubyte, uint8_t)
DEFINE_DUMP_TYPE_HEADER(int, int)

#undef DEFINE_DUMP_TYPE_HEADER

void gltf_node_parse(GLTFFile *file, Model *model, NodeIndex n_idx);
void gltf_materials_parse(GLTFFile *file, Model *model);
void gltf_animations_parse(GLTFFile *file, Model *model);
// parse directly into a list of buffer pointers on the model.
void gltf_images_parse(GLTFFile *file, Model *model);
// parse into a list of opengl texture ids with the right sampler applied.
void gltf_textures_parse(GLTFFile *file, Model *model);

GLTF_ComponentType gltf_get_accessor_ct(GLTFFile *file, int accessor_index);
