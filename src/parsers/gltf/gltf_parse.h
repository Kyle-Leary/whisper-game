#pragma once

#include "defines.h"
#include "wjson.h"

typedef struct GLTFFile {
  // header and section header defined stuff
  u32 version;
  u32 gltf_length;

  // json and binary gltf chunks can be in either order.

  u32 binary_offset;
  u32 binary_length;
  u8 *binary_data;

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
  WJSONValue *buffer_views;
  WJSONValue *buffers;
  WJSONValue *skins;
  WJSONValue *animations;
  WJSONValue *cameras;
  WJSONValue *extensions;
  WJSONValue *extras;
} GLTFFile;

GLTFFile *gltf_parse(const char *file_path);
