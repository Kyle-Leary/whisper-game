#pragma once

#include "backends/graphics_api.h"

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

extern RenderConfigurationData render_config_table[RC_COUNT];
