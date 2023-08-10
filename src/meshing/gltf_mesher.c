#include "gltf_mesher.h"
#include "helper_math.h"
#include "parsers/gltf/gltf_parse.h"
#include "util.h"
#include "wjson.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

// hardcode this to return a RC_BASIC, most default exports from 3d software
// will have the same vertex attributes, that is position -> normal -> uv.
GraphicsRender *gltf_to_render_simple(GLTFFile *file) {
  printf("making render from GLTFFile structure %p.\n", file);

  uint n_verts, n_idx;

  // VERY IMPORTANT TO USE UNSIGNED SHORT HERE, OR ELSE THE DATA WON'T BE
  // INTERPRETED RIGHT!! IBO IN GLB ARE SHORTS, NOT UINTS.
  unsigned short indices[2048];

  float positions[2048]; // 3 components for each vertex position
  float normals[2048];
  float uvs[2048]; // 2 components for each UV coordinate

  // grab the first mesh.
  WJSONValue *mesh = wjson_index(file->meshes, 0);
  WJSONValue *prims = wjson_get(mesh, "primitives");

  PRINT_PTR(prims)

  // grab the first prim from the first mesh.
  WJSONValue *prim = wjson_index(prims, 0);

  PRINT_PTR(prim)

  WJSONValue *attributes =
      wjson_get(prim, "attributes"); // all the vertex attributes of the
                                     // vertices in this primitive.

  PRINT_PTR(attributes)

  // these attribute names will always be the same, and give us info on where
  // the actual data for this attribute of this prim is in the binary buffer.
  //
  // the attributes point to an accessor, which points to a binaryView into the
  // GLB binary chunk.
  // these return doubles as indices by default, since they're js number types.
  // valuecast them to ints and use them as indices.
  int pos_accessor_idx = (int)wjson_number(wjson_get(attributes, "POSITION"));
  int norm_accessor_idx = (int)wjson_number(wjson_get(attributes, "NORMAL"));
  int uv_accessor_idx = (int)wjson_number(wjson_get(attributes, "TEXCOORD_0"));

  int idx_accessor_idx = (int)wjson_number(
      wjson_get(prim, "indices")); // where is the index buffer?

  // TODO: mats? mat index is right next to indices, so it would be parsed out
  // here.

  WJSONValue *pos_accessor = wjson_index(file->accessors, pos_accessor_idx);
  WJSONValue *norm_accessor = wjson_index(file->accessors, norm_accessor_idx);
  WJSONValue *uv_accessor = wjson_index(file->accessors, uv_accessor_idx);
  WJSONValue *idx_accessor = wjson_index(file->accessors, idx_accessor_idx);

  // assume that there are uniform vertices across each buffer type.
  n_verts = wjson_number(wjson_get(
      pos_accessor,
      "count")); // use the pos accessor count for the total vertex count.
  n_idx = wjson_number(
      wjson_get(idx_accessor, "count")); // indices are the only ones that have
                                         // consistently different count.

  // then, load in the buffers from the binary segments using the accessor
  // specification.

  // get the bufferViews that tell us exactly how each buffer is laid out in
  // memory.

  uint pos_bv_idx = (uint)wjson_number(wjson_get(pos_accessor, "bufferView"));
  uint idx_bv_idx = (uint)wjson_number(wjson_get(idx_accessor, "bufferView"));

  // then, parse out the data from the bufferViews.

  int pos_bv_len = gltf_bv_get_len(file, pos_bv_idx);
  gltf_bv_parse(file, pos_bv_idx, positions, pos_bv_len);
  printf("glb position buffer length: %d bytes.\n", pos_bv_len);
  int idx_bv_len = gltf_bv_get_len(file, idx_bv_idx);
  printf("glb index buffer length: %d bytes.\n", idx_bv_len);
  gltf_bv_parse(file, idx_bv_idx, indices, idx_bv_len);

  // dumb value cast of an array. we need ushort -> uint for gl compatibility.
  unsigned int converted_indices[n_idx];
  for (int i = 0; i < n_idx; i++) {
    converted_indices[i] = (unsigned int)indices[i];
  }

  GraphicsRender *gr =
      g_new_render((VertexData *)&(BasicVertexData){RC_BASIC, n_verts,
                                                    positions, normals, uvs},
                   converted_indices, n_idx);

  return gr;
}
