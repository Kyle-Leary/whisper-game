#pragma once

#include "parsers/gltf/gltf_parse.h"
#include "render/model.h"

Model *gltf_to_model(GLTFFile *file);

// just take the first mesh and prim from that mesh, turn that into a render.
GraphicsRender *gltf_to_render_simple(GLTFFile *file);
