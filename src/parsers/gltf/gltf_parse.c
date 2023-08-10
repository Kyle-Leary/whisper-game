#include "gltf_parse.h"

#include "defines.h"
#include "size.h"
#include "util.h"
#include "wjson.h"
#include "wjson/src/defines.h"
#include "wjson/src/util.h"
#include <string.h>

#define MAX_GLTF_SZ (MB(5))

// TODO: this is all assuming a little endian system.

// parse the json chunk from the string directly into the file structure.
static void parse_json_chunk(GLTFFile *file, char *json_str) {
  WJSONValue *root = wjson_parse_string(json_str);

#define GET(which)                                                             \
  {                                                                            \
    file->which = wjson_get(root, #which);                                     \
    if (!file->which) {                                                        \
      printf("WARN: the parsed JSON chunk of the glb file doesn't have the "   \
             "top-level parameter " #which ".\n");                             \
    }                                                                          \
  }

  GET(asset);
  GET(scenes);
  GET(nodes);
  GET(meshes);
  GET(materials);
  GET(textures);
  GET(images);
  GET(samplers);
  GET(accessors);
  GET(bufferViews);
  GET(buffers);
  GET(skins);
  GET(animations);
  GET(cameras);
  GET(extensions);
  GET(extras);

#undef GET
}

GLTFFile *gltf_parse(const char *file_path) {
  // pass in the literal to this macro, and it will check if the magic number is
  // correct at the current index into the input buffer, and bump past if it is.
#define CHECK_MAGIC(magic_num_string)                                          \
  {                                                                            \
    if (memcmp(input, magic_num_string, 4) != 0) {                             \
      input[4] = '\0';                                                         \
      fprintf(stderr,                                                          \
              "ERROR: returning NULL gltf file, " magic_num_string             \
              " magic number "                                                 \
              "does not match with the provided string %s.\n",                 \
              input);                                                          \
      return NULL;                                                             \
    }                                                                          \
    input += 4;                                                                \
  }

  GLTFFile *file = (GLTFFile *)malloc(sizeof(GLTFFile) * 1);

  char input_buffer[MAX_GLTF_SZ];
  char *input = (char *)
      input_buffer; // now, we can treat this like a normal ptr and do
                    // incrementing arithmetic to bump it through the buffer.

  read_file_data(file_path, input_buffer, MAX_GLTF_SZ);

  CHECK_MAGIC("glTF")

  printf("past gltf magic\n");

  file->version = *(u32 *)(input);
  input += 4; // bump past the version number.

  file->gltf_length = *(u32 *)(input);
  input += 4; // bump past the full gltf length spec in bytes.

  printf("past full gltf header\n");

  // SECTION HEADER - length, type then raw data output.

  // i think there are some cases where the gltf file doesn't have both, but
  // those aren't useful to us right now. just assume that it'll both have a
  // JSON and BIN section at some point in the file.
  for (int i = 0; i < 2; i++) {
    // parse a generic header, figure out the type and act accordingly later.
    u32 header_len = *(u32 *)(input);
    input += 4;

    printf("parsing the %dth section header\n", i);

    // switch over the first letter of the magic, if it's J, try to parse JSON,
    // if it's B, try to parse BIN.
    switch (*input) { // deref, it's still a u8 char pointer internally.
    case 'J': {
      CHECK_MAGIC("JSON")
      file->json_length = header_len;
      char json_data[file->json_length];
      memcpy(json_data, input, file->json_length);
      json_data[file->json_length] = '\0';
      parse_json_chunk(file, json_data);
      input += file->json_length;
      // then, we're done!

    } break;
    case 'B': {
      CHECK_MAGIC("BIN\0")
      file->binary_length = header_len;
      file->binary_data = (u8 *)malloc(file->binary_length);
      memcpy(file->binary_data, input, file->binary_length);
      PRINT_PTR(file->binary_data)
      input += file->binary_length;

    } break;
    default: {
      error("Invalid GLB section header. Found section magic starting "
            "character %c.",
            *input);
    } break;
    }
  }

  {
    // then, read out all the buffer offsets and malloc THAT array.
    file->num_buffers = file->buffers->data.length.array_len;
    file->buffer_offsets = (u32 *)malloc(file->num_buffers * sizeof(u32));
    file->buffer_offsets[0] = 0; // start at zero offset.
    for (int i = 1; i < file->num_buffers; i++) {
      // recursively sum the buffer offsets.
      file->buffer_offsets[i] =
          file->buffer_offsets[i - 1] +
          (u32)wjson_number(
              wjson_get(wjson_index(file->buffers, i), "byteLength"));
    }
  }

  return file;

#undef CHECK_MAGIC
}

/* EXAMPLE BUFFERVIEW:
 *{"buffer":0,"byteLength":288,"byteOffset":0,"target":34962},{"buffer":0,"byteLength":288,"byteOffset":288,"target":34962},{"buffer":0,"byteLength":192,"byteOffset":576,"target":34962},{"buffer":0,"byteLength":72,"byteOffset":768,"target":34963}
 * */
int gltf_bv_get_len(GLTFFile *file, int index) {
  WJSONValue *bv = wjson_index(file->bufferViews, index);
  int length = (int)wjson_number(wjson_get(bv, "byteLength"));
  return length;
}

void gltf_bv_parse(GLTFFile *file, int index, void *dest, int dest_sz) {
  // this function will return a malloced ptr. i want to pass in the stack
  // alloced buffer, but there's no easy way to determine the sizeof the buffer
  // without parsing.
  WJSONValue *bv = wjson_index(file->bufferViews, index);
  u32 buf_idx = wjson_number(wjson_get(bv, "buffer"));
  u32 byte_len = wjson_number(wjson_get(bv, "byteLength"));
  u32 byte_offset = wjson_number(wjson_get(bv, "byteOffset"));
  void *data_ptr =
      (file->binary_data + file->buffer_offsets[buf_idx] + byte_offset);
  memcpy(dest, data_ptr, byte_len);
}
