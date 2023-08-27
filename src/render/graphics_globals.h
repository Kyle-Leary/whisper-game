#include "../ogl_includes.h"
#include "whisper/hashmap.h"

// also share state between the backend globally here.
extern GLFWwindow *window;

// reference each shader with a human-readable string.
extern WHashMap shader_map;

extern GLuint bone_data_ubo;
extern GLuint material_data_ubo;
