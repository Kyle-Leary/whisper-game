#pragma once
// g_ is the api namespace for the graphics api.

#include "cglm/types.h"
#include "main.h"
#include <stdbool.h>
#include <sys/types.h>

// a handle to a texture, an id can always be rep'd as a uint, likely.
typedef uint TextureHandle;

extern TextureHandle textures[NUM_TEXTURES];

// returns an index into the global textures array, after loading it into the
// graphics backend.
uint g_load_texture(const char *filepath);
// "activates" the texture, using it for further draw calls.
// TODO: how to reason about texture slots?
void g_use_texture(TextureHandle handle);

// abstract over shaders by allowing selection between multiple pipeline
// configurations, all of which must be somehow set and filled through the
// graphics backend selected.
// for example; in modern gl, a pipelineconfig might result in a useProgram call
// and a shader compilation. on the n64, this might result in some gl calls and
// rdp configurations.
typedef enum PipelineConfiguration {
  PC_BASIC,
  PC_HUD,
  PC_COUNT,
} PipelineConfiguration;

// the render configuration data table is handled entirely by the backend. these
// are just the frontend enums
typedef enum RenderConfiguration {
  RC_BASIC,
  RC_HUD,
  RC_COUNT,
} RenderConfiguration;

// a general Material structure, materials can be selected at any time and
// affect the rendering process in a platform-specific way.
typedef struct Material {
  // goes by the PBR spec generally laid out by the glb format.
  vec4 albedo;
  float metallic;
  float roughness;
  TextureHandle base_color_texture;
  TextureHandle metallic_roughness_texture;
  TextureHandle normal_texture;
  TextureHandle occlusion_texture;
  TextureHandle emissive_texture;
  vec3 emissive_factor;
  bool double_sided;
} Material;

// all renders from now on will use the mat Material.
// all shaders are implicitly PBR shaders?
void g_use_material(Material *mat);

/* define light structures that will be passed directly into a PBR-type shader.
 */

#define POINT_LIGHT_SLOTS 5
#define SPOT_LIGHT_SLOTS 5
#define DIRECTIONAL_LIGHT_SLOTS 5

// an index into a light array, operating as a "light slot", since the lights
// are simply stored as elements in a global array.
typedef int LightSlot;

typedef struct SpotLight {
  vec3 position;
} SpotLight;

typedef struct PointLight {
  vec3 position;
  vec4 color;
  float intensity;
  float range;
} PointLight;

typedef struct DirectionalLight {
  vec3 direction;
  vec4 color;
  float intensity;
} DirectionalLight;

// this is the odd one out, only one instance. a kind of global illumination in
// the scene.
typedef struct AmbientLight {
  vec4 color;
  float intensity;
} AmbientLight;

// the global structure that holds all the light data in the scene. one of these
// should be active at a time, and it should generally be managed through helper
// methods.
typedef struct LightData {
  // shaders can only store and pass around fixed-width data, so we need this
  // all structured at compile-time. that's the point of "slots" rather than
  // variable-sized data in a shader, it's just how cpu gpu communication works.

  SpotLight spot_lights[SPOT_LIGHT_SLOTS];
  // how many slots are currently active? we fill the slots from left to right,
  // and make things as easy as possible on the GPU. the glsl shader is dumb, it
  // can't know that much about the shape or context of this data.
  int n_spot_lights;

  PointLight point_lights[DIRECTIONAL_LIGHT_SLOTS];
  int n_point_lights;

  DirectionalLight directional_lights[POINT_LIGHT_SLOTS];
  int n_directional_lights;

  AmbientLight ambient_light;
} LightData;

extern LightData g_light_data;

// we need to be able to remove the light later, so let the caller manage the
// lightslot. worst comes to worst, we can just flush the light data.
LightSlot g_add_point_light(PointLight *light); // copy into the newest slot
void g_remove_point_light(LightSlot slot);

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

// extensions of the VertexData structure
typedef struct BasicVertexData {
  VERTEX_DATA_FIELDS

  // these are seperate pointers to the different allocated segments of data
  // that point to position, normal, uv etc. these are NOT NOT NOT!!!
  // interleaved arrays.
  float *position;
  float *normal;
  float *uv;
} BasicVertexData;

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
  RenderConfiguration conf;

  Render *internal; // the opaque parts.
} GraphicsRender;

void g_init();

/* pass the g_new_render function one of the *VertexData structure type
 * pointers. it will be resolved and matched based on the configuration. */
GraphicsRender *g_new_render(VertexData *data, const unsigned int *indices,
                             unsigned int i_count);
/* draw a render, under the context of the specified pipeline settings. when
 * passed NULL, the behavior should be simply doing nothing and printing some
 * sort of error. */
void g_draw_render(GraphicsRender *r);

void g_clean();
