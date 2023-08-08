#pragma once
// g_ is the api namespace for the graphics api.

#include "cglm/types.h"
#include "main.h"
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
