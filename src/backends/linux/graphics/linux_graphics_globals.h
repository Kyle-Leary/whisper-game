#include "../ogl_includes.h"
#include "backends/linux/graphics/shader.h"
#include "whisper/hashmap.h"

// also share state between the backend globally here.
extern GLFWwindow *window;

// reference each shader with a human-readable string.
extern WHashMap shader_map;

extern Shader *curr_program;

// only one Material can be bound at a time.
// generally, Materials are processed one mesh at a time.
extern Material *curr_mat;

// store bone_data ubo block in slot 2.
#define BONE_BLOCK 2

extern GLuint bone_data_ubo;
