#include "../ogl_includes.h"
#include "backends/linux/graphics/shader.h"

// also share state between the backend globally here.
extern GLFWwindow *window;

extern Shader *basic_program;
extern Shader *hud_program;
extern Shader *gouraud_program;

extern Shader *curr_program;

// only one Material can be bound at a time.
// generally, Materials are processed one mesh at a time.
extern Material *curr_mat;
