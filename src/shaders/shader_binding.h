#pragma once

#include "cglm/types.h"
#include "shader_instances.h"

void shader_bind(Shader *s);
void shader_handle_model(Shader *s, mat4 model);
