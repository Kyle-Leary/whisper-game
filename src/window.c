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

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  // Set the OpenGL viewport to cover the entire window
  glViewport(0, 0, width, height);
  win_w = width;
  win_h = height;
}

// dumb
static void save_screenshot(GLFWwindow *window, const char *filename) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  // Allocate space for the pixels
  GLubyte *pixels = (GLubyte *)malloc(width * height * 4 * sizeof(GLubyte));
  if (!pixels) {
    fprintf(stderr, "Failed to allocate memory for screenshot\n");
    return;
  }

  // Read the pixels from the current frame buffer
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // Flip the image vertically, as OpenGL's origin is in the lower-left
  int channels = 4; // RGBA
  GLubyte *flipped_pixels = (GLubyte *)malloc(width * height * channels);
  if (!flipped_pixels) {
    fprintf(stderr, "Failed to allocate memory for flipped image\n");
    free(pixels);
    return;
  }

  for (int y = 0; y < height; y++) {
    memcpy(&flipped_pixels[y * width * channels],
           &pixels[(height - y - 1) * width * channels], width * channels);
  }

  // Save the pixel data to a PNG file
  if (!stbi_write_png(filename, width, height, channels, flipped_pixels,
                      width * channels)) {
    fprintf(stderr, "Failed to save screenshot to %s\n", filename);
  }

  free(pixels);
  free(flipped_pixels);
}

// Function to handle GLFW errors
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

#ifdef DEBUG
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam) {
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
  glClearColor(0.1F, 0.1F, 0.3F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void window_update() {
  glfwPollEvents(); // call this before the input update, this signals the
                    // callbacks.
  if (i_state.act_just_pressed[ACT_SCREENSHOT]) {
    save_screenshot(window, "screen.png");
  }
}

void window_end_draw() { glfwSwapBuffers(window); }

void window_clean() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
