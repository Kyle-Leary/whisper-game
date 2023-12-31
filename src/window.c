#include "cglm/mat4.h"
#include "global.h"
#include "input/input.h"
#include "macros.h"
#include "main.h"
#include "ogl_includes.h"
#include "path.h"
#include "util.h"
#include "whisper/hashmap.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

GLFWwindow *window = NULL;

void window_force_close() { glfwSetWindowShouldClose(window, GLFW_TRUE); }

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  // Set the OpenGL viewport to cover the entire window
  glViewport(0, 0, width, height);
  win_w = width;
  win_h = height;
}

int window_get_frame_buffer_size() { return win_w * win_h * 4; }

void window_get_frame_buffer(byte *buf) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  // Read the pixels from the current frame buffer
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
}

// Function to handle GLFW errors
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

#ifdef DEBUG
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam) {
  // don't clog up stderr with the "memory mapping busy, stalled..." message.
  if (type == 0x8250)
    return;

  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity,
          message);
}
#endif /* ifdef DEBUG */

void window_init() {
  // Initialize GLFW
  if (!glfwInit()) {
    ERROR_NO_ARGS("Failed to initialize GLFW\n");
  }

  // Set GLFW error callback
  glfwSetErrorCallback(error_callback);

  // window hints before creating the window.

  // Create a windowed mode window and its OpenGL context
  window =
      glfwCreateWindow(win_w, win_h, WIN_TITLE, NULL,
                       NULL); // set the global window, so that the other /linux
                              // backend implementations like "input" and
                              // "graphics" can access this windowstate.
  if (!window) {
    glfwTerminate();
    ERROR_NO_ARGS("Failed to create GLFW window\n");
  }

  // hide the cursor
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Make the window's context current
  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Initialize GLEW
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    glfwDestroyWindow(window);
    glfwTerminate();
    char buf[512];
    sprintf(buf, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
    ERROR_FROM_BUF(buf);
  }

#ifdef DEBUG
  glEnable(GL_DEBUG_OUTPUT); // use the newer debug output, instead of
  // manually handling the error stack from the gpu.
  glDebugMessageCallback(MessageCallback, 0);

  { // print out some limits.
    GLint maxUniforms;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxUniforms);
    printf("Max uniforms supported: %d.\n", maxUniforms);

    GLint max_size;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_size);
    printf("Max ubo block size supported: %d.\n", max_size);

    GLint max_attributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attributes);
    printf("Max vertex attribs supported: %d.\n", max_attributes);

    GLint max_vertex_uniform_blocks;
    GLint max_fragment_uniform_blocks;
    GLint max_geometry_uniform_blocks;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &max_vertex_uniform_blocks);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &max_fragment_uniform_blocks);
    glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &max_geometry_uniform_blocks);
    printf("Max Vertex Uniform Blocks: %d\n", max_vertex_uniform_blocks);
    printf("Max Fragment Uniform Blocks: %d\n", max_fragment_uniform_blocks);
    printf("Max Geometry Uniform Blocks: %d\n", max_geometry_uniform_blocks);
  }
#endif /* ifdef DEBUG */
}

bool window_should_close() { return glfwWindowShouldClose(window); }

void window_begin_draw() {
  glClearColor(0.03F, 0.03F, 0.03F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void window_update() {
  glfwPollEvents(); // call this before the input update, this signals the
                    // callbacks.
}

void window_end_draw() { glfwSwapBuffers(window); }

void window_clean() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
