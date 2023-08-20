#include "../ogl_includes.h"
#include "backends/graphics/shader.h"
#include "whisper/hashmap.h"

// also share state between the backend globally here.
extern GLFWwindow *window;

// reference each shader with a human-readable string.
extern WHashMap shader_map;

extern Shader *curr_program;

// store bone_data ubo block in slot 2.
#define BONE_BLOCK 2
#define MATERIAL_BLOCK 3

extern GLuint bone_data_ubo;
extern GLuint material_data_ubo;
