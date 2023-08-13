#include "backends/graphics_api.h"
#include "backends/linux/graphics/shader.h"
#include "global.h"
#include "linux_graphics_globals.h"
#include "main.h"
#include "util.h"
#include "vao.h"
#include <string.h>
#include <sys/types.h>

#include "../ogl_includes.h"
#include <stdlib.h>

#include <cglm/cglm.h>
#include <cglm/types.h>

#include "render_structs.h"

// we'll define the type locally here. think of the graphics_api.h header as the
// "common" headerfile for the linux/graphics/*.c files. it'll share all the
// public graphics symbols together in one file, since the api is fairly simple.

// everything needed to render a "thing"
typedef struct Render {
  uint vao; // we only need to explicitly include the VAO, the rest of the
            // buffers are bound to it.

  uint n_idx; // number of indices of the bound vbo to the vao that we'll
              // actually render from the bound ebo.
} Render;

// not static, we need to make vbos in the vao vertexdata-type-specific
// configuration function.
uint make_vbo(const void *data, unsigned int count,
              unsigned int sizeof_vtx) { // the size of the vertex is required
                                         // to calculate the buffer data size.

  // "count" as in the literal amount of datapoints/floats going into the vbo.

  uint render_id;
  glGenBuffers(1, &render_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_id);
  // printf("Making a VBO with data size %d.\n", sizeof_vtx * count);
  // printf("Making a VBO from pointer %p.\n", data);
  glBufferData(GL_ARRAY_BUFFER, sizeof_vtx * count, data,
               GL_STATIC_DRAW); // we could actually get away with passing this
                                // stack-allocated vertices, since we're calling
                                // this IN the new_Render function anyway?
  return render_id;
}

static uint make_ebo(const unsigned int *indices, unsigned int count) {
  uint render_id;
  glGenBuffers(1, &render_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices,
               GL_STATIC_DRAW);
  return render_id;
}

static uint make_vao() {
  uint render_id;
  glGenVertexArrays(1, &render_id); // make the vao
  glBindVertexArray(
      render_id); // note; each of the make_ functions in this source file
                  // implicitly bind the object in the context.
  return render_id;
}

// the data is dumped into vbo and ebo slots, and the vbo and ebo are associated
// with a vao. when we draw, we simply draw arrays with the vao bound.
GraphicsRender *g_new_render(VertexData *data, const unsigned int *indices,
                             unsigned int i_count) {
  Render *r = (Render *)malloc(sizeof(Render));
  r->n_idx = i_count;

  // WE NEED THE VAO BOUND BEFORE WE DO ANYTHING!!!
  // we can't just bind the vao later, even though the vertex attribute setter
  // calls are supposed to actually do the linking between the two.
  r->vao = make_vao(); // bind the vao first

  { // make the buffers next.

    make_ebo(indices, i_count); // the ebo is just one, it's the same for each.

    // the config function will take care of taking in the data pointer, casting
    // it properly and then making all the right vbos for each attribute of the
    // vertex data. once we've done this once, it's bound to the vao forever and
    // we don't have to worry about the data anymore.
    render_config_table[data->conf].conf_func(data);
  }

  // unbind the vertex array, then delete the buffers.
  glBindVertexArray(0);

  /// TODO: for freeing, verify that this actually works. don't want to do this
  /// until i'm sure it doesn't crash immediately.
  ///
  // deletion of the actual buffers will be deferred when "they're not in use".
  // glDeleteBuffers(2, render_ids); // batch the operation, delete both at the
  // same time from the stack-allocated array.

  {
    // then, make the actual return structure with the internal Render
    // substructure.
    GraphicsRender *gr = (GraphicsRender *)malloc(sizeof(GraphicsRender));
    gr->internal =
        r; // this can't be literal, it needs to be a pointer. we
           // reference and reason about opaque types through pointers
           // only, since we don't know their size or layout.
    gr->conf = data->conf;

    glm_mat4_identity(
        gr->model); // each graphics render has a model offset that
                    // can be easily modified through the cglm api,
                    // no matter what backend you target.

    // use whatever default configuration works for this, the caller can change
    // this if they want. i don't think there's a point in actually having a
    // setter method for something like this, they can just change the value
    // directly.
    gr->pc = g_default_pc(gr->conf);

    return gr;
  }
}

// mutate the model per-object.
void g_draw_render(GraphicsRender *gr) {
  if (gr == NULL) {
    fprintf(
        stderr,
        "ERROR: [g_draw_render()] Could not render NULL GraphicsRender*.\n");
  }

  // TODO: somehow use the curr_mat PBR Material in this render? pass the props
  // to the shader? how can we abstract over shaders and shader uniforms
  // cleanly?

  glBindVertexArray(gr->internal->vao);

  // use the attached pipeline for the render, ideally the backend will handle
  // this caching and optimization on its own, so that we won't have
  // redundancies in graphics calls for binding and etc.
  g_use_pipeline(gr->pc);

  // after a pipeline switch, bind the model to the current shader every single
  // model will have a shader matrix, no matter what. the view and projection
  // are handled in a UBO elsewhere, ideally set in a loop every frame.
  shader_set_matrix4fv(curr_program, "model", (const float *)gr->model);
  glDrawElements(GL_TRIANGLES, gr->internal->n_idx, GL_UNSIGNED_INT, 0);
}

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
        fprintf(stderr, "ERROR: Could not find an armature in the GLB model on "
                        "trying to g_draw_model().\n");
        exit(1);
      }
    }

    Skin *skin = m->nodes[arm].data.skin;

    mat4 tfs[BONE_LIMIT];

    int num_bones = skin->num_joints;

    for (int i = 0; i < num_bones; i++) {
      // copy the ith joint into the ith transform slot, setting up the BoneData
      // transforms to be sent over to the GPU.
      memcpy(&tfs[i], &(skin->joints[i]), sizeof(float) * 16);
    }

    g_use_bones(tfs, num_bones);
  }

  {} // TODO: mats

  // then, draw the render.
  g_draw_render(m->render);
}

// maybe setup and use shaders/uniforms here instead of the lifecycle? it really
// doesn't matter.
void g_init() {}

void g_clean() {}
