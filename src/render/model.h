#pragma once

#include "animation/anim_struct.h"
#include "cglm/types.h"
#include "defines.h"
#include "render/graphics_render.h"
#include "render/material.h"
#include "render/rigging.h"
#include <stdint.h>
#include <sys/types.h>

typedef enum NodeType {
  NT_INVALID = 0,
  NT_ARMATURE,
  NT_BONE,
  NT_MESH,
  NT_COUNT,
} NodeType;

// negatives are invalid values? idk
typedef int16_t NodeIndex;

// list of indices to other nodes, as opposed to lists of pointers.
// we're just saying fuck it and coupling the structure with the glb file, makes
// things way easier.
typedef struct Node {
  NodeType type;
  NodeIndex parent;
  NodeIndex *children;
  int num_children;
  // we need to pass direct pointers to these props one-by-one from the
  // animation system, so it's better to have them as seperate vectors rather
  // than pre-composing them into a matrix and having that be their primary
  // internal representation.
  vec3 translation;
  vec4 rotation;
  vec3 scale;

  mat4 transform_calc; // space to calculate the transform. don't rely on this
                       // to be updated.

  union {
    Skin *skin; // for the armature type.
  } data;
} Node;

#define INVALID_NODE -1

#define MAX_IMAGES 32
#define MAX_TEXTURES 32
#define MAX_PRIMITIVES 32

typedef struct Image {
  byte *buf;
  int len;
} Image;

typedef struct Primitive {
  GraphicsRender *render;
  int material_idx; // the index into the model's materials array that this prim
                    // is using.
} Primitive;

// assuming a typical glb with one scene in it.
typedef struct Model {
  Animation *animations;
  int num_animations;

  // a glb scene can have multiple top-level nodes in it.
  NodeIndex *roots;
  int num_roots;

  // at render-time, traverse the node tree and grab all of the bone data for
  // use in the tree. the Node structures contain all the necessary local bone
  // transformations.
  Node *nodes;
  int num_nodes; // in a glb, we need to index into arrays to get anywhere, so
                 // it's not extremely productive to organize nodes as a tree.
                 // just make them an array, especially since we'll always know
                 // the length at parse-time.

  // parse out a list of materials into the Model.
  Material *materials;
  int num_materials;

  // a model can render in multiple different segments with different shaders.
  Primitive primitives[MAX_PRIMITIVES];
  int num_primitives;

  // point to a list of image buffers.
  Image images[MAX_IMAGES];
  int num_images;

  // textures are just loaded images with samplers applied. they're a layer
  // between image data and its representation in the 3d world.
  uint textures[MAX_TEXTURES];
  int num_textures;

  // a general purpose transform applied to all the child transforms of the
  // primitives rendered by this Model.
  mat4 transform;
} Model;

void g_draw_model(Model *m);
