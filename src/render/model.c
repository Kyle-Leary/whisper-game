#include "model.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "render/rigging.h"

#include "macros.h"

// search, ideally from the root of the model recursively for the first Armature
// node type we find. in this Node design, an "Armature"-type node is just one
// that happens to have a "skin" field in the node description.
static NodeIndex arm_search(Model *m, NodeIndex idx) {
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
      arm_idx = arm_search(m, node->children[i]);
      if (arm_idx != INVALID_NODE) {
        return arm_idx;
      }
    }
  }

  // didn't find the armature.
  return INVALID_NODE;
}

// go through and update the bone data, find the right materials and update
// those, THEN render the internal graphics render after binding the right
// model rendering shader.
void g_draw_model(Model *m) {
  { // update bone data based on the Model's new bone positions.
    NodeIndex arm = INVALID_NODE;
    { // hacky, just try to find the first armature in the node hierarchy.
      for (int i = 0; i < m->num_roots; i++) {
        arm = arm_search(m, m->roots[i]);
        if (arm != INVALID_NODE) {
          break;
        }
      }
      if (arm == INVALID_NODE) {
        ERROR_NO_ARGS("ERROR: Could not find an armature in the GLB model on "
                      "trying to g_draw_model().\n");
      }
    }

    Skin *skin = m->nodes[arm].data.skin;

    mat4 tfs[BONE_LIMIT];

    int num_bones = skin->num_joints;

    for (int i = 0; i < num_bones; i++) {
      mat4 bone_tf;
      glm_mat4_identity(bone_tf);

      int joint_node_index = skin->joints[i];
      // copy the ith joint into the ith transform slot, setting up the BoneData
      // transforms to be sent over to the GPU.
      Node *bone_node = &(m->nodes[joint_node_index]);

      // generate the transform cpu-side for now from the bone Node* structure.
      glm_translate(bone_tf, bone_node->translation);
      glm_scale(bone_tf, bone_node->scale);
      glm_quat_rotate(bone_tf, bone_node->rotation, bone_tf);

      // then copy the transform into the slot.
      memcpy(&tfs[i], bone_tf, sizeof(float) * 16);
    }

    rig_use_bones(tfs, skin->ibms, num_bones);
  }

  {} // TODO: materials

  mat4 temp_modelmat;

  for (int i = 0; i < m->num_renders; i++) {
    GraphicsRender *curr_render = m->render[i];
    glm_mat4_copy(curr_render->model, temp_modelmat);
    glm_mat4_mul(curr_render->model, m->transform, curr_render->model);
    g_draw_render(curr_render);
    // restore the state of the old matrix before the transformation.
    glm_mat4_copy(temp_modelmat, curr_render->model);
  }
}
