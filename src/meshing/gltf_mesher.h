#pragma once

#include "parsers/gltf/gltf_parse.h"
#include "render/model.h"

#define VERT_BUF_SZ KB(50)

Model *gltf_to_model(GLTFFile *file);

// just take the first mesh and prim from that mesh, turn that into a render.
GraphicsRender *gltf_to_render_simple(GLTFFile *file);
