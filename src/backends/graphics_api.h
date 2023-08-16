#pragma once
// g_ is the api namespace for the graphics api.

#include "animation/anim_struct.h"
#include "cglm/types.h"
#include "main.h"
#include "whisper/contig_array.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

// a handle to a texture, an id can always be rep'd as a uint, likely.
typedef uint TextureHandle;

extern TextureHandle textures[NUM_TEXTURES];

// certain higher special texture slots for subsystems that need their own
// texture and don't change it that often, so it makes sense to batch all calls
// to that texture and keep it loaded.
#define FONT_TEX_SLOT 7
#define SKYBOX_TEX_SLOT 8

void g_set_font(TextureHandle handle);

void g_set_font_color(vec3 color);

// returns an index into the global textures array, after loading it into the
// graphics backend.
uint g_load_texture(const char *filepath);
uint g_load_cubemap(char *faces[6]);
// "activates" the texture, using it for further draw calls.
// TODO: how to reason about texture slots?
void g_use_texture(TextureHandle handle, int slot);
void g_use_cubemap(TextureHandle handle, int slot);

// abstract over shaders by allowing selection between multiple pipeline
// configurations, all of which must be somehow set and filled through the
// graphics backend selected.
// for example; in modern gl, a pipelineconfig might result in a useProgram call
// and a shader compilation. on the n64, this might result in some gl calls and
// rdp configurations.
typedef enum PipelineConfiguration {
  PC_INVALID = 0,

  PC_BASIC,

  // define some flat HUD rendering styles.
  PC_HUD,

  // define HUD text rendering modes, each with different effects that can be
  // applied to text.
  PC_HUD_TEXT, // draws from the special font texture slot.
  PC_HUD_TEXT_WAVY,

  PC_WIREFRAME,

  PC_SKYBOX,

  PC_BLANK_GOURAUD, // does a blank gouraud shader over the ambient coloring.
  PC_PBR_GOURAUD,   // gouraud with PBR in the vs.
  PC_SOLID,         // renders a single color from the uniforms.

  PC_MODEL, // renders a model with pbr lightdata and bones.
  PC_COUNT,
} PipelineConfiguration;

// the render configuration data table is handled entirely by the backend. these
// are just the frontend enums
typedef enum RenderConfiguration {
  RC_BASIC,
  RC_HUD,
  RC_MODEL, // rigged model. it's a superset of the RC_BASIC, but with ivec4
            // joint indices and vec4 weights. we pass in JOINT_ and WEIGHT_
            // through the attributes, and pass in the bone data through a UBO.
  RC_COUNT,
} RenderConfiguration;

// PBR material structure, this is similarly slotted directly into the block UBO
// data.
typedef struct MaterialData {
  // goes by the PBR spec generally laid out by the glb format.
  vec4 albedo;
  float metallic;
  char padding1[12];
  float roughness;
  char padding2[12];
  vec3 emissive_factor;
  char padding8[4];
  bool double_sided;
  char padding9[12];
} MaterialData;

// have a wrapper structure that can double as both the raw UBO data and
// something more expressive, without having to pointer-chase to pull the
// structure back together.
typedef struct ModelMaterial {
  MaterialData inner;

  bool double_sided;
} ModelMaterial;

// all renders from now on will use the mat Material.
// all shaders are implicitly PBR shaders?
void g_use_material(MaterialData *mat);

typedef enum TextureType {
  // these enum variant values double as the slots that the textures are
  // inserted into.
  TEX_BASE_COLOR = 0,
  TEX_METALLIC_ROUGHNESS = 1,
  TEX_NORMAL = 2,
  TEX_OCCLUSION = 3,
  TEX_EMISSIVENESS = 4,
  TEX_COUNT,
} TextureType;

void g_use_pbr_texture(TextureType type, TextureHandle tex);

// a maximum of 128 bones. this should be enough for our simpler animations.
#define BONE_LIMIT 128

// the data that will be sent directly to the GPU.
typedef struct BoneData {
  mat4 ibms[BONE_LIMIT];
  mat4 bones[BONE_LIMIT];
  // similarly, fill up the bones contiguously and dictate the maximum to the
  // gpu.
  int num_bones;
  char padding[12];
} BoneData;

// will be copied into the right place, on linux this copies into the ubo for
// rendering.
void g_use_bones(mat4 *bones, mat4 *ibms, int num_bones);

/* define light structures that will be passed directly into a PBR-type shader.
 */

#define POINT_LIGHT_SLOTS 5
#define SPOT_LIGHT_SLOTS 5
#define DIRECTIONAL_LIGHT_SLOTS 5

// an index into a light array, operating as a "light slot", since the lights
// are simply stored as elements in a global array.
typedef uint LightSlot;

typedef struct SpotLight {
  vec3 position;
  char pad1[4];
} SpotLight;

typedef struct PointLight {
  vec3 position;
  char pad1[4];
  vec4 color;
  // vec4 is already padded
  float intensity;
  float range;
  char pad2[8];
} PointLight;

typedef struct DirectionalLight {
  vec3 direction;
  char pad1[4];
  vec4 color;
  float intensity;
  char pad2[12];
} DirectionalLight;

// this is the odd one out, only one instance. a kind of global illumination in
// the scene.
typedef struct AmbientLight {
  vec4 color;
  float intensity;
  char pad1[12];
} AmbientLight;

// 12 bytes of padding at the end of the contig array, the W_CA_PointLight
// structure will be stuck directly in the light data.
DEFINE_CONTIG_ARRAY_TYPES(PointLight, POINT_LIGHT_SLOTS, , , char padding[12];)
DEFINE_CONTIG_ARRAY_TYPES(DirectionalLight, DIRECTIONAL_LIGHT_SLOTS, , ,
                          char padding[12];)
DEFINE_CONTIG_ARRAY_TYPES(SpotLight, SPOT_LIGHT_SLOTS, , , char padding[12];)

// the global structure that holds all the light data in the scene. one of these
// should be active at a time, and it should generally be managed through helper
// methods.
typedef struct LightData {
  // shaders can only store and pass around fixed-width data, so we need this
  // all structured at compile-time. that's the point of "slots" rather than
  // variable-sized data in a shader, it's just how cpu gpu communication works.

  // these MUST BE IN THE RIGHT ORDER. BE VERY CAREFUL WITH THE LAYOUT OF THIS
  // STRUCTURE, IT IS EXTREMELY DELIBERATE.
  W_CA_SpotLight spot_light_ca;
  W_CA_PointLight point_light_ca;
  W_CA_DirectionalLight directional_light_ca;

  AmbientLight ambient_light;
} LightData;

// in the shader, the structure is exactly the same so we don't need to worry
// about drilling past the ContiguousArray stuff in the glsl code/structs.

extern LightData g_light_data;

/* v_count - the amount of vertices (actual points) in the data. not the amount
 of floats or bytes. */
#define VERTEX_DATA_FIELDS                                                     \
  RenderConfiguration conf;                                                    \
  unsigned int v_count;

// information that will be used to bind the data. in most formats, the data
// comes seperated rather than interleaved, so this is more practical.
typedef struct VertexData {
  VERTEX_DATA_FIELDS
} VertexData;

#define BASIC_VERTEX_DATA_FIELDS                                               \
  VERTEX_DATA_FIELDS                                                           \
  float *position;                                                             \
  float *normal;                                                               \
  float *uv;

// extensions of the VertexData structure
typedef struct BasicVertexData {
  BASIC_VERTEX_DATA_FIELDS
} BasicVertexData;

typedef struct ModelVertexData {
  BASIC_VERTEX_DATA_FIELDS

  int *joint; // four joint indices per vertex in a basic model.
  float *weight;
} ModelVertexData;

typedef struct HUDVertexData {
  VERTEX_DATA_FIELDS

  float *position;
  float *uv;
} HUDVertexData;

void g_use_pipeline(PipelineConfiguration config);

// expose the opaque Render type, let the apis have unique data for each.
typedef struct Render Render;

typedef struct GraphicsRender {
  // the stuff that's common with all the graphics apis.
  mat4 model;

  RenderConfiguration conf; // the basic vertex type that the render is using

  PipelineConfiguration
      pc; // the pipeline configuration/shader that the render is using.
          // sticking this on the GraphicsRender isn't too much slower and makes
          // things considerably simpler.

  Render *internal; // the opaque parts.
} GraphicsRender;

typedef enum NodeType {
  NT_INVALID = 0,
  NT_ARMATURE,
  NT_BONE,
  NT_MESH,
  NT_COUNT,
} NodeType;

// assume a model of animation and scene hierarchy not unlike a glb file.
typedef struct Node Node;

// negatives are invalid values? idk
typedef int16_t NodeIndex;

// a skin has "joints" which is a list of Node indices in the global glb node
// array.
typedef struct Skin {
  // all the inverse bind matrices for every single bone in the armature
  // referencing this Skin.
  mat4 *ibms;
  int *joints;
  // one joint index and ibm for each joint in the skin.
  int num_joints;
} Skin;

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
  union {
    Skin *skin; // for the armature type.
  } data;
} Node;

#define INVALID_NODE -1

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
  ModelMaterial *materials;
  int num_materials;

  // a render contains just the constant vertex buffers bound to the vao, it's
  // nothing specific to rendering.
  GraphicsRender *render;

} Model;

void g_init();

// the default config associated with a render configuration. not platform
// specific. with this, we won't have to think about setting the right pc every
// single time.
PipelineConfiguration g_default_pc(RenderConfiguration conf);

/* pass the g_new_render function one of the *VertexData structure type
 * pointers. it will be resolved and matched based on the configuration. */
GraphicsRender *g_new_render(VertexData *data, const unsigned int *indices,
                             unsigned int i_count);

/* draw a render, under the context of the specified pipeline settings. when
 * passed NULL, the behavior should be simply doing nothing and printing some
 * sort of error. */
void g_draw_render(GraphicsRender *r);

void g_draw_model(Model *m);

void g_clean();

// might as well make this an enum. no overhead, really.
typedef enum DepthMode {
  DM_OFF,
  DM_ON,
  DM_COUNT,
} DepthMode;

/* now, define general pipeline settings: */
void g_set_depth_mode(DepthMode dm);
