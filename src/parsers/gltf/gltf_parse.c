#include "gltf_parse.h"

#include "defines.h"
#include "size.h"
#include "util.h"
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
  GET(buffer_views);
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
      // then, we're done!

    } break;
    case 'B': {
      CHECK_MAGIC("BIN\0")
      file->binary_length = header_len;
      memcpy(file->binary_data, input, file->binary_length);

    } break;
    }
  }

  return file;

#undef CHECK_MAGIC
}
