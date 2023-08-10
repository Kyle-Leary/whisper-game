#pragma once

#include "defines.h"
#include "wjson.h"

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

int gltf_bv_get_len(GLTFFile *file, int index);
void gltf_bv_parse(GLTFFile *file, int index, void *dest,
                   int dest_sz); // write the data into dest ptr
                                 // specified by the bufferView in the file at
                                 // the specified index into the bufferView
                                 // toplevel array. determine the proper size of
                                 // the dest ptr with gltf_bv_get_len.
