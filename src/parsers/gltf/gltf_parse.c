#include "gltf_parse.h"

#include "animation/anim_struct.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "defines.h"
#include "meshing/gltf_mesher.h"
#include "parsers/gltf/wjson_help.h"
#include "printers.h"
#include "render/model.h"
#include "render/texture.h"
#include "size.h"
#include "util.h"
#include "wjson.h"
#include "wjson/src/defines.h"
#include "wjson/src/util.h"
#include <stdint.h>
#include <string.h>

#include "ogl_includes.h"

#define MAX_GLTF_SZ (MB(5))

// TODO: this is all assuming a little endian system.

static WJSONValue v_blank_array;

// parse the json chunk from the string directly into the file structure.
static void parse_json_chunk(GLTFFile *file, char *json_str) {
  WJSONValue *root = wjson_parse_string(json_str);

  // default to a safe blank array for all of the fields. i think all top-level
  // values in a glb JSON chunk are arrays?
  v_blank_array.data.value.array = NULL;
  v_blank_array.data.length.array_len = 0;

#define GET(which)                                                             \
  {                                                                            \
    file->which = wjson_get(root, #which);                                     \
    if (!file->which) {                                                        \
      file->which = &v_blank_array;                                            \
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

  file->version = *(u32 *)(input);
  input += 4; // bump past the version number.

  file->gltf_length = *(u32 *)(input);
  input += 4; // bump past the full gltf length spec in bytes.

  // SECTION HEADER - length, type then raw data output.

  // i think there are some cases where the gltf file doesn't have both, but
  // those aren't useful to us right now. just assume that it'll both have a
  // JSON and BIN section at some point in the file.
  for (int i = 0; i < 2; i++) {
    // parse a generic header, figure out the type and act accordingly later.
    u32 header_len = *(u32 *)(input);
    input += 4;

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

void gltf_file_free(GLTFFile *file) { free(file); }

/* EXAMPLE BUFFERVIEW:
 *{"buffer":0,"byteLength":288,"byteOffset":0,"target":34962},{"buffer":0,"byteLength":288,"byteOffset":288,"target":34962},{"buffer":0,"byteLength":192,"byteOffset":576,"target":34962},{"buffer":0,"byteLength":72,"byteOffset":768,"target":34963}
 * */
int gltf_bv_get_len(GLTFFile *file, int index) {
  WJSONValue *bv = wjson_index(file->bufferViews, index);
  int length = (int)wjson_number(wjson_get(bv, "byteLength"));
  return length;
}

void gltf_bv_parse(GLTFFile *file, int index, void *dest) {
  WJSONValue *bv = wjson_index(file->bufferViews, index);
  u32 buf_idx = wjson_number(wjson_get(bv, "buffer"));
  u32 byte_len = wjson_number(wjson_get(bv, "byteLength"));
  u32 byte_offset = wjson_number(wjson_get(bv, "byteOffset"));
  void *data_ptr =
      (file->binary_data + file->buffer_offsets[buf_idx] + byte_offset);
  memcpy(dest, data_ptr, byte_len);
}

// dumping the data out of an accessor is just a matter of copying the buffer
// data into the array of the right type, so we can act generically over any
// accessor and have the caller specify the type.
#define DEFINE_DUMP_TYPE(name, type_name)                                      \
  int gltf_dump_##name##_accessor(GLTFFile *file, int accessor_idx,            \
                                  type_name *buffer, int *n_elms) {            \
    WJSONValue *accessor = wjson_index(file->accessors, accessor_idx);         \
    *n_elms = wjson_number(wjson_get(accessor, "count"));                      \
    uint bv_idx = (uint)wjson_number(wjson_get(accessor, "bufferView"));       \
    int bv_len = gltf_bv_get_len(file, bv_idx);                                \
    gltf_bv_parse(file, bv_idx, buffer);                                       \
    return bv_len;                                                             \
  }

DEFINE_DUMP_TYPE(float, float)
DEFINE_DUMP_TYPE(ushort, unsigned short)
DEFINE_DUMP_TYPE(ubyte, uint8_t)
DEFINE_DUMP_TYPE(int, int)

#undef DEFINE_DUMP_TYPE

void gltf_node_parse(GLTFFile *file, Model *model, NodeIndex n_idx) {
  WJSONValue *v_nodes = file->nodes;

  NodeIndex target_node_index = n_idx;
  Node *target_node = &(model->nodes[target_node_index]);

  WJSONValue *v_node = wjson_index(v_nodes, target_node_index);

  do { // setup the type of the node. this is a do..while(0) so that we can
       // execute it once and break; out of the block whenever we'd like.

    // we're determining the type based on what properties the node has. if
    // it has a skin, it's an Armature.
    WJSONValue *v_skin_idx = wjson_get(v_node, "skin");

    if (v_skin_idx) { // then parse an armature type out of the node.
      int skin_idx = wjson_number(
          v_skin_idx); // we get an index into the glb top-level skins array.

      target_node->type = NT_ARMATURE;
      Skin *skin = malloc(sizeof(Skin));
      // now, parse the skin. break these up into seperate parts later, this
      // function sucks

      // get the skin json object that the armature node was referencing.
      WJSONValue *v_skin = wjson_index(file->skins, skin_idx);

      // the list of joints, joints are just special cases of Nodes.
      WJSONValue *v_joints = wjson_get(v_skin, "joints");

      skin->num_joints = v_joints->data.length.array_len;
      skin->joints =
          malloc(sizeof(int) * skin->num_joints); // joints is an array of ints.
      for (int j = 0; j < skin->num_joints; j++) {
        // copy into the array.
        skin->joints[j] = (int)wjson_number(wjson_index(v_joints, j));
      }

      // then, parse out the ibms from the accessor.
      skin->ibms =
          malloc(sizeof(float) * 16 *
                 skin->num_joints); // each IBM is just a normal transform
                                    // matrix, and is sized as such.
      int ibm_acc_idx = wjson_number(wjson_get(v_skin, "inverseBindMatrices"));
      int dumb;
      gltf_dump_float_accessor(file, ibm_acc_idx, (float *)skin->ibms, &dumb);

      target_node->data.skin = skin;
      break;
    }

    // by default? i'm not sure. not everything can be determined in this
    // way, which is a big problem actually.
    target_node->type = NT_BONE;
  } while (0);

  WJSONValue *children = wjson_get(v_node, "children");
  if (!children) {
    // no "children" attribute on the node, this is a leaf.
    target_node->num_children = 0;
    target_node->children = NULL;
  } else { // then, setup the children of the node.
    target_node->num_children = children->data.length.array_len;
    // oh god freeing the Model struct is going to be a fucking nightmare.
    // what is this pointer chasing bullshit
    target_node->children =
        malloc(sizeof(NodeIndex) * target_node->num_children);

    for (int j = 0; j < target_node->num_children; j++) {
      NodeIndex child_node_index =
          (NodeIndex)wjson_number(wjson_index(children, j));

      target_node->children[j] = child_node_index;

      // have the parent iterator manually set the parent on the
      // child. i can't think of an easier way to do this.
      model->nodes[child_node_index].parent = target_node_index;
    }
  }

  { // now, init the basic properties of the node, i guess depending on the type
    // of the node?

    // try to parse the optional prop fields, and fall back on generic ones if
    // not found.
    WJSONValue *v_translation = wjson_get(v_node, "translation");
    if (v_translation) {
      for (int i = 0; i < 3; i++) {
        target_node->translation[i] =
            wjson_number(wjson_index(v_translation, i));
      }
    } else {
      glm_vec3_zero(target_node->translation);
    }

    WJSONValue *v_rotation = wjson_get(v_node, "rotation");
    if (v_rotation) {
      for (int i = 0; i < 4; i++) {
        target_node->rotation[i] = wjson_number(wjson_index(v_rotation, i));
      }
    } else {
      memcpy(&(target_node->rotation), (versor){0, 0, 0, 1}, sizeof(float) * 4);
    }

    WJSONValue *v_scale = wjson_get(v_node, "scale");
    if (v_scale) {
      for (int i = 0; i < 3; i++) {
        target_node->scale[i] = wjson_number(wjson_index(v_scale, i));
      }
    } else {
      glm_vec3_one(target_node->scale);
    }
  }
}

// TODO: fix this strncmp bullshit!!!
InterpolationType gltf_interpolation_type_parse(const char *interp_name) {
  if (strncmp(interp_name, "LINEAR", 6) == 0) {
    return IP_LINEAR;
  } else if (strncmp(interp_name, "STEP", 4) == 0) {
    return IP_STEP;
  } else {
    return IP_INVALID;
  }
}

void gltf_samplers_parse(GLTFFile *file, WJSONValue *v_samplers,
                         Sampler *samplers, int num_samplers) {
  float buffer[5096];

  for (int i = 0; i < num_samplers; i++) {
    WJSONValue *v_sampler = wjson_index(v_samplers, i);
    Sampler *sampler = &(samplers[i]);
    sampler->interp = gltf_interpolation_type_parse(
        wjson_string(wjson_get(v_sampler, "interpolation")));

    int input_accessor_idx = wjson_number(wjson_get(v_sampler, "input"));
    int input_sz = gltf_dump_float_accessor(file, input_accessor_idx, buffer,
                                            &sampler->num_frames);
    sampler->input = malloc(input_sz);
    memcpy(sampler->input, buffer, input_sz);

    int dumb;
    int output_accessor_idx = wjson_number(wjson_get(v_sampler, "output"));
    int output_sz =
        gltf_dump_float_accessor(file, output_accessor_idx, buffer, &dumb);
    sampler->output = malloc(output_sz);
    memcpy(sampler->output, buffer, output_sz);
  }
}

// literally link the samplers to the channels with a pointer for ease of
// access.
void gltf_channels_parse(GLTFFile *file, WJSONValue *v_channels,
                         Sampler *samplers, Channel *channels,
                         int num_channels) {
  for (int i = 0; i < num_channels; i++) {
    WJSONValue *v_channel = wjson_index(v_channels, i);

    Channel *channel = &(channels[i]);
    int sampler_index = wjson_number(wjson_get(v_channel, "sampler"));
    channel->sampler = &(samplers[sampler_index]); // linked

    // just parse the node and property directly, we won't handle the property
    // system stuff directly in the parsing.
    WJSONValue *v_target = wjson_get(v_channel, "target");
    WJSONValue *v_prop_name = wjson_get(v_target, "path");
    int prop_name_len = v_prop_name->data.length.str_len;
    char *prop_name = wjson_string(v_prop_name);
    channel->target.property_name = malloc(prop_name_len);
    memcpy(channel->target.property_name, prop_name, prop_name_len);
    channel->target.node_index = wjson_number(wjson_get(v_target, "node"));

    channel->channel_end_time =
        channel->sampler->input[channel->sampler->num_frames - 1];
  }
}

// parse gltffile into the list of ModelMaterials internal to the Model.
void gltf_materials_parse(GLTFFile *file, Model *model) {
  WJSONValue *v_materials = file->materials;

  int num_mats = v_materials->data.length.array_len;

  model->num_materials = num_mats;
  model->materials = malloc(sizeof(ModelMaterial) * model->num_materials);

  for (int i = 0; i < num_mats; i++) {
    ModelMaterial *curr_mat = &(model->materials[i]);
    WJSONValue *v_material = wjson_index(v_materials, i);

    // try to parse out the baseColorTexture nested ID from the material.
    JSON_TRY_GET(pbrMetallicRoughness, v_material, {
      JSON_TRY_GET(baseColorTexture, v_pbrMetallicRoughness,
                   {JSON_TRY_GET(index, v_baseColorTexture, {
                     JSON_NUMBER(v_index);
                     curr_mat->base_color_texture =
                         model->textures[(int)num_v_index];
                   })});
    });

    curr_mat->double_sided =
        (wjson_get(v_material, "doubleSided")->data.value.boolean);
  }
}

// parse all of the animations from the GLTFFile* to the Model*.
void gltf_animations_parse(GLTFFile *file, Model *model) {
  WJSONValue *v_animations = file->animations;
  int num_anims = v_animations->data.length.array_len;

  model->num_animations = num_anims;
  model->animations = malloc(sizeof(Animation) * model->num_animations);

  for (int i = 0; i < num_anims; i++) {
    Animation *curr_anim = &(model->animations[i]);

    WJSONValue *v_animation = wjson_index(v_animations, i);

    WJSONValue *v_channels = wjson_get(v_animation, "channels");
    WJSONValue *v_samplers = wjson_get(v_animation, "samplers");
    WJSONValue *v_name = wjson_get(v_animation, "name"); // optional

    { // handle the optional name.
      if (!v_name) {
        fprintf(stderr, "ERROR: animations without names in the GLB file are "
                        "currently unsupported.\n");
        exit(1);
      } else {
        curr_anim->name = wjson_string(v_name);
      }
    }

    { // samplers
      curr_anim->num_samplers = v_samplers->data.length.array_len;
      curr_anim->samplers = malloc(sizeof(Sampler) * curr_anim->num_samplers);
      gltf_samplers_parse(file, v_samplers, curr_anim->samplers,
                          curr_anim->num_samplers);
    }

    { // channels
      curr_anim->num_channels = v_channels->data.length.array_len;
      curr_anim->channels = malloc(sizeof(Channel) * curr_anim->num_channels);
      gltf_channels_parse(file, v_channels, curr_anim->samplers,
                          curr_anim->channels, curr_anim->num_channels);
    }
  }
}

void gltf_images_parse(GLTFFile *file, Model *model) {
  WJSONValue *v_images = file->images;
  int num_images = v_images->data.length.array_len;

  RUNTIME_ASSERT_MSG(
      num_images < MAX_IMAGES,
      "increase MAX_IMAGES, or maybe something just went wrong.");

  model->num_images = num_images;

  for (int i = 0; i < num_images; i++) {
    WJSONValue *v_image = wjson_index(v_images, i);
    WJSONValue *v_bufview = wjson_get(v_image, "bufferView");
    int image_data_idx = (int)v_bufview->data.value.number;

    int len = gltf_bv_get_len(file, image_data_idx);
    u8 *buf = malloc(len);
    // write directly from the bv into the buffer, now we have the image data.
    gltf_bv_parse(file, image_data_idx, buf);

    // store the POINTER in the model, there's no real way to store the data
    // directly when it's this variable-length.
    model->images[i].buf = buf;
    model->images[i].len = len;
  }
}

void gltf_textures_parse(GLTFFile *file, Model *model) {
  WJSONValue *v_textures = file->textures;
  int num_textures = v_textures->data.length.array_len;

  RUNTIME_ASSERT_MSG(
      num_textures < MAX_TEXTURES,
      "increase MAX_TEXTURES, or maybe something just went wrong.");

  model->num_textures = num_textures;

  for (int i = 0; i < num_textures; i++) {
    WJSONValue *v_texture = wjson_index(v_textures, i);
    // how should we modify the texture's loading? sampler turns directly into
    // opengl texture sampling attribute calls
    WJSONValue *v_sampler = wjson_get(v_texture, "sampler");
    // where should the png image buffer come from?
    WJSONValue *v_source = wjson_get(v_texture, "source");

    int sampler_idx = (int)wjson_number(v_sampler);
    int source_idx = (int)wjson_number(v_source);

    // take a copy, we just need the source param to read this.
    Image im = model->images[source_idx];
    model->textures[i] = g_load_texture_from_png_buf(im.buf, im.len);

    {
      // then, parse the sampler into gl texture attrib calls.
      // the texture should still be bound from the above g_load_texture_...
      // call.
      JSON_INDEX(sampler_idx, file->samplers);
      JSON_TRY_GET(magFilter, v_sampler_idx, {
        JSON_NUMBER(v_magFilter);
        // glb encodes the opengl enum variants directly, so we can just dump
        // them directly into the call.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, num_v_magFilter);
      });

      JSON_TRY_GET(minFilter, v_sampler_idx, {
        JSON_NUMBER(v_minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, num_v_minFilter);
      });

      JSON_TRY_GET(wrapS, v_sampler_idx, {
        JSON_NUMBER(v_wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, num_v_wrapS);
      });

      JSON_TRY_GET(wrapT, v_sampler_idx, {
        JSON_NUMBER(v_wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, num_v_wrapT);
      });
    }
    // by now, the texture should be ready for use in other materials.
  }
}

// componentType is specifically a property of an accessor.
GLTF_ComponentType gltf_get_accessor_ct(GLTFFile *file, int accessor_index) {
  WJSONValue *accessors = file->accessors;
  return wjson_number(
      wjson_get(wjson_index(accessors, accessor_index), "componentType"));
}
