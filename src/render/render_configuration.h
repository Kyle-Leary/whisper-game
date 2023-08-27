#pragma once

#include <sys/types.h>

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

// the render table data is private, but this allows you to get a VAO id
// configured from a raw data pointer. this is the big function that turns
// VertexData* -> vao, and it works polymorphically over the type of vertexdata
// you pass in.
// this does not RETURN the vao, it simply configures the vao properly with the
// vertex buffer data passed in. you need to have a vao (optionally an ebo as
// well) bound BEFORE you call this function.
void configure_render(VertexData *data);

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

// this is backend-implemenation specific.
#define SIZEOF_BASIC_VTX (sizeof(float) * 8)
#define SIZEOF_MODEL_VTX (sizeof(float) * 16)
#define SIZEOF_HUD_VTX (sizeof(float) * 4)

// a blank function that runs through the full configuration of a VAO for the
// creation of VAOs in the render_structs.c file.
typedef void (*VAOConfiguration)(
    void *); // pass it some vertex data, infer by the type of vertex.

// this is the local definition, the api specifies that there is a list of
// renderconfigurations for each render type. the api has an opaque
// RenderConfigurationData.
typedef struct RenderConfigurationData {
  unsigned int sizeof_vtx; // size of the vertices in this particular rendering
                           // configuration
  VAOConfiguration conf_func; // the function that configures the vao's attrib
                              // structure properly.
} RenderConfigurationData;
