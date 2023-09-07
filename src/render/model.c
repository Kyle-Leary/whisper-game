#include "model.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "global.h"
#include "im_prims.h"
#include "math/mat.h"
#include "printers.h"
#include "render/graphics_render.h"
#include "render/material.h"
#include "render/rigging.h"

#include "macros.h"
#include "render/texture.h"
#include "shaders/shader_instances.h"
#include "whisper/macros.h"
#include <string.h>
#include <sys/types.h>

// search, ideally from the root of the model recursively for the first Armature
// node type we find. in this Node design, an "Armature"-type node is just one
// that happens to have a "skin" field in the node description.
static NodeIndex _arm_search(Model *m, NodeIndex idx) {
  NodeIndex arm_idx = INVALID_NODE;
  Node *node = &(m->nodes[idx]);

  if (node->type == NT_ARMATURE) {
    // base case, we've reached the bottom.
    arm_idx = idx;
    return arm_idx;
  } else {
    if (node->children == NULL || node->num_children == 0) {
      return INVALID_NODE;
    }
    // iter through all the children of this node, and check for armatures.
    for (int i = 0; i < node->num_children; i++) {
      arm_idx = _arm_search(m, node->children[i]);
      if (arm_idx != INVALID_NODE) {
        return arm_idx;
      }
    }
  }

  // didn't find the armature.
  return INVALID_NODE;
}

// this is generic, update the internal transform of this node and all child
// nodes. call the base case with tf as the identity matrix, probably for some
// dumbass reason i made the Node structure hold its children in indices, not
// pointers so we also need to pass in the whole list here to get child node
// references.
static void _update_node_tf(Node *node_list, NodeIndex root_idx, mat4 tf) {
  Node *node_root = &node_list[root_idx];

  mat4_identity(node_root->transform_calc);
  // then compose this transform with the parent's transform.
  glm_mat4_mul(node_root->transform_calc, tf, node_root->transform_calc);

  // generate the transform cpu-side for now from the node Node* structure.
  glm_translate(node_root->transform_calc, node_root->translation);
  glm_quat_rotate(node_root->transform_calc, node_root->rotation,
                  node_root->transform_calc);
  glm_scale(node_root->transform_calc, node_root->scale);

  for (int i = 0; i < node_root->num_children; i++) {
    _update_node_tf(node_list, node_root->children[i],
                    node_root->transform_calc);
  }
}

// go through and update the bone data, find the right materials and update
// those, THEN render the internal graphics render after binding the right
// model rendering shader.
void g_draw_model(Model *m) {
  { // update bone data based on the Model's new bone positions.
    NodeIndex arm = INVALID_NODE;

    { // hacky, just try to find the first armature in the node hierarchy.
      for (int i = 0; i < m->num_roots; i++) {
        arm = _arm_search(m, m->roots[i]);
        if (arm != INVALID_NODE) {
          break;
        }
      }
      if (arm == INVALID_NODE) {
        ERROR_NO_ARGS("ERROR: Could not find an armature in the GLB model on "
                      "trying to g_draw_model().\n");
      }
    }

    BoneData bones;

    Skin *skin = m->nodes[arm].data.skin;

    int num_bones = skin->num_joints;

    {
      mat4 id;
      for (int i = 0; i < m->num_roots; i++) {
        mat4_identity(id); // just in case, make sure its identity every root.
        // update each root, since we're about to use some node transform data.
        _update_node_tf(m->nodes, m->roots[i], id);
      }
    }

    vec3 last_tr;

    for (int i = 0; i < num_bones; i++) {
      int joint_node_index = skin->joints[i];
      // copy the ith joint into the ith transform slot, setting up the BoneData
      // transforms to be sent over to the GPU.
      Node *bone_node = &(m->nodes[joint_node_index]);

      // glm_mat4_mul(bone_node->transform_calc, skin->ibms[i],
      //              bone_node->transform_calc);

      mat4 tf;
      glm_mat4_identity(tf);

      glm_mat4_mul(tf, skin->ibms[i], tf);
      glm_mat4_mul(tf, bone_node->transform_calc, tf);

      // printf("=== (IBM) %d\n", i);
      // print_mat4(skin->ibms[i], 0);
      // printf("===\n");
      //
      // printf("=== %d\n", i);
      // print_mat4(tf, 0);
      // printf("===\n");

      im_transform(skin->ibms[i], 0.2);
      im_transform(bone_node->transform_calc, 1);

      { // column major, extract tx ty and tz.
        vec3 tr;
        tr[0] = bone_node->transform_calc[3][0]; // x
        tr[1] = bone_node->transform_calc[3][1]; // y
        tr[2] = bone_node->transform_calc[3][2];
        im_point(tr);

        if (i > 0) {
          im_vector(last_tr, tr, (vec4){1, 1, 1, 1});
        }

        memcpy(last_tr, tr, 12);
      }

      // then copy the transform into the slot.
      memcpy(&(bones.bones[i]), tf, sizeof(float) * 16);
    }

    bones.num_bones = num_bones;

    rig_use_bones(&bones);
  }

  mat4 temp_modelmat;

  for (int i = 0; i < m->num_primitives; i++) {
    Primitive prim = m->primitives[i];
    int mat_idx = prim.material_idx;
    if (mat_idx != -1) {
      // if the prim is associated with a material, set that material up before
      // rendering.
      Material mat = m->materials[mat_idx];

      // set the render up with the external material context.
      uint texture_id = mat.base_color_texture;
      g_use_texture(texture_id, 0);
    }

    GraphicsRender *curr_render = prim.render;

    // then setup the model matrix, offset it by the model's absolute transform.
    glm_mat4_copy(curr_render->model, temp_modelmat);
    glm_mat4_mul(curr_render->model, m->transform, curr_render->model);

    // then render it.
    g_draw_render(curr_render);

    // restore the state of the old matrix before the transformation.
    glm_mat4_copy(temp_modelmat, curr_render->model);
  }
}
