#include "gltf_mesher.h"
#include "backends/graphics_api.h"
#include "helper_math.h"
#include "parsers/gltf/gltf_parse.h"
#include "printers.h"
#include "util.h"
#include "wjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// pass in alloced arrays, and this will modify based on the GLTFFile*'s
// internal state.
static WJSONValue *parse_basic_attrs(GLTFFile *file, float *positions,
                                     float *normals, float *uvs,
                                     unsigned int *indices, int *n_verts,
                                     int *n_idx) {
  unsigned short
      internal_indices[2048]; // will be transformed into the int* index buffer.

  // grab the first mesh.
  WJSONValue *mesh = wjson_index(file->meshes, 0);
  WJSONValue *prims = wjson_get(mesh, "primitives");

  // grab the first prim from the first mesh.
  WJSONValue *prim = wjson_index(prims, 0);

  // the caller might need to keep parsing attributes, so pass this out of the
  // function with a pointer.
  WJSONValue *attributes =
      wjson_get(prim, "attributes"); // all the vertex attributes of
                                     // the vertices in this primitive.

  int pos_accessor_idx = (int)wjson_number(wjson_get(attributes, "POSITION"));
  gltf_dump_float_accessor(file, pos_accessor_idx, positions, n_verts);

  int norm_accessor_idx = (int)wjson_number(wjson_get(attributes, "NORMAL"));
  gltf_dump_float_accessor(file, norm_accessor_idx, normals, n_verts);

  int uv_accessor_idx = (int)wjson_number(wjson_get(attributes, "TEXCOORD_0"));
  gltf_dump_float_accessor(file, uv_accessor_idx, uvs, n_verts);

  int idx_accessor_idx = (int)wjson_number(wjson_get(prim, "indices"));
  gltf_dump_ushort_accessor(file, idx_accessor_idx, internal_indices, n_idx);

  for (int i = 0; i < *n_idx; i++) {
    // handle the conversion IN the actual function itself.
    // directly into the passed indices*
    indices[i] = (unsigned int)internal_indices[i];
  }

  return attributes;
}

Model *gltf_to_model(GLTFFile *file) {
  Model *model = malloc(sizeof(Model));

  int n_verts, n_idx;
  float positions[2048], normals[2048], uvs[2048], weights[2048];
  // then, the model-specific data.
  int joints[2048]; // these are actually ivec4s, not weird unsigned shorts like
                    // the index data.
  unsigned int indices[2048];

  WJSONValue *attributes = parse_basic_attrs(file, positions, normals, uvs,
                                             indices, &n_verts, &n_idx);

  // then, use the attributes table to parse the extra Model stuff, like weights
  // and joints.
  WJSONValue *v_wai = wjson_get(attributes, "WEIGHTS_0");
  int weight_accessor_idx = (int)wjson_number(v_wai);
  gltf_dump_float_accessor(file, weight_accessor_idx, weights, &n_verts);

  int joint_accessor_idx = (int)wjson_number(wjson_get(attributes, "JOINTS_0"));
  gltf_dump_int_accessor(file, joint_accessor_idx, joints, &n_verts);

  GraphicsRender *gr = g_new_render(
      (VertexData *)&(ModelVertexData){RC_MODEL, n_verts, positions, normals,
                                       uvs, joints, weights},
      indices, n_idx);

  gr->pc = PC_MODEL;

  { // more JSON parsing for Model data not explicitly related to vertex
    // attributes/data.

    { // parse all of the nodes on their own.
      WJSONValue *v_nodes = file->nodes;
      int num_nodes = v_nodes->data.length.array_len;
      model->nodes = malloc(sizeof(Node) * num_nodes);
      model->num_nodes = num_nodes;

      // iterate through the nodes and parse each of them.
      for (int i = 0; i < num_nodes; i++) {
        // parse the node at the current index INTO the Model* structure.
        gltf_node_parse(file, model, i);
      }
    }

    { // parse all the top-level root nodes out of the 0th scene. assume the GLB
      // file won't use another scene, for now.
      WJSONValue *v_scene_zero = wjson_index(file->scenes, 0);
      WJSONValue *v_scene_zero_nodes = wjson_get(v_scene_zero, "nodes");
      model->num_roots = v_scene_zero_nodes->data.length.array_len;
      model->roots = malloc(sizeof(int) * model->num_roots);

      for (int i = 0; i < model->num_roots; i++) {
        // get all the top level node indices in the first scene.

        NodeIndex root_index =
            (NodeIndex)wjson_number(wjson_index(v_scene_zero_nodes, i));
        model->roots[i] = root_index;
        // set top level nodes to have an INVALID parent.
        model->nodes[root_index].parent = INVALID_NODE;
      }
    }

    model->num_animations = file->animations->data.length.array_len;

    gltf_animations_parse(file, model);

    model->render = gr;
  }

  free(file);

  return model;
}

// hardcode this to return a RC_BASIC, most default exports from 3d software
// will have the same vertex attributes, that is position -> normal -> uv.
GraphicsRender *gltf_to_render_simple(GLTFFile *file) {
  printf("making render from GLTFFile structure %p.\n", file);

  int n_verts, n_idx;

  unsigned int indices[2048];
  float positions[2048], normals[2048], uvs[2048];

  WJSONValue *attributes = parse_basic_attrs(file, positions, normals, uvs,
                                             indices, &n_verts, &n_idx);

  GraphicsRender *gr =
      g_new_render((VertexData *)&(BasicVertexData){RC_BASIC, n_verts,
                                                    positions, normals, uvs},
                   indices, n_idx);

  free(file);

  return gr;
}
