#pragma once
// g_ is the api namespace for the graphics api.

#include "cglm/types.h"
#include "main.h"
#include "whisper/contig_array.h"
#include <stdbool.h>
#include <sys/types.h>

// a handle to a texture, an id can always be rep'd as a uint, likely.
typedef uint TextureHandle;

extern TextureHandle textures[NUM_TEXTURES];

// returns an index into the global textures array, after loading it into the
// graphics backend.
uint g_load_texture(const char *filepath);
uint g_load_cubemap(char *faces[6]);
// "activates" the texture, using it for further draw calls.
// TODO: how to reason about texture slots?
void g_use_texture(TextureHandle handle);
void g_use_cubemap(TextureHandle handle);

// abstract over shaders by allowing selection between multiple pipeline
// configurations, all of which must be somehow set and filled through the
// graphics backend selected.
// for example; in modern gl, a pipelineconfig might result in a useProgram call
// and a shader compilation. on the n64, this might result in some gl calls and
// rdp configurations.
typedef enum PipelineConfiguration {
  PC_BASIC,
  PC_HUD,
  PC_SKYBOX,
  PC_BLANK_GOURAUD, // does a blank gouraud shader over the ambient coloring.
  PC_PBR_GOURAUD,   // gouraud with PBR in the vs.
  PC_SOLID,         // renders a single color from the uniforms.
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

// might as well make this an enum. no overhead, really.
typedef enum DepthMode {
  DM_OFF,
  DM_ON,
  DM_COUNT,
} DepthMode;

/* now, define general pipeline settings: */
void g_set_depth_mode(DepthMode dm);
