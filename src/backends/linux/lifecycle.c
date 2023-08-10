#include "backends/input_api.h"
#include "backends/lifecycle_api.h"

#include "backends/linux/graphics/shader.h"
#include "global.h"
#include "main.h"
#include "ogl_includes.h"
#include "path.h"
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// define the externs in ogl_includes.h
GLFWwindow *window = NULL;

int loc_model = 0;
int loc_view_rot = 0;
int loc_view_tf = 0;
int loc_projection = 0;

int loc_main_slot = 0;
int loc_u_time = 0;

int loc_ui_model = 0;
int loc_ui_projection = 0;
int loc_ui_u_time = 0;

int loc_ui_font_slot = 0;
int loc_ui_text_color = 0;

GLuint basic_program = 0;
GLuint hud_program = 0;

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

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam) {
  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity,
          message);
}

int l_init() {
  // Initialize GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  // Set GLFW error callback
  glfwSetErrorCallback(error_callback);

  // Create a windowed mode window and its OpenGL context
  window =
      glfwCreateWindow(WIN_W, WIN_H, WIN_TITLE, NULL,
                       NULL); // set the global window, so that the other /linux
                              // backend implementations like "input" and
                              // "graphics" can access this windowstate.
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  // hide the cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  glEnable(GL_DEBUG_OUTPUT); // use the newer debug output, instead of
  // manually handling the error stack from the gpu.
  glDebugMessageCallback(MessageCallback, 0);

  GLint maxUniforms;
  glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxUniforms);
  printf("Max uniforms supported: %d.\n", maxUniforms);

  // the "default" shader for normal 3d scenes
  basic_program = make_shader(SHADER_PATH("basic.vs"), SHADER_PATH("basic.fs"));
  glUseProgram(basic_program);

  // we don't need to regrab them everytime we bind, they're dependent on the
  // existence of the program itself, not the binding. so we can only do this
  // once, right at the start of the application.
  loc_model = glGetUniformLocation(basic_program, "model");
  loc_view_rot = glGetUniformLocation(basic_program, "view_rot");
  loc_view_tf = glGetUniformLocation(basic_program, "view_tf");
  loc_projection = glGetUniformLocation(basic_program, "projection");

  loc_u_time = glGetUniformLocation(basic_program, "u_time");
  loc_main_slot = glGetUniformLocation(basic_program, "main_slot");

  // set the main shader uniforms while that's bound.
  glUniform1i(loc_main_slot, 0); // use the 0th texture slot for basic tris

  // the default for the hud rendering, eg text stuff. we don't really want to
  // render the hud in perspective.
  hud_program = make_shader(SHADER_PATH("hud.vs"), SHADER_PATH("hud.fs"));

  loc_ui_model = glGetUniformLocation(hud_program, "model");
  loc_ui_projection = glGetUniformLocation(hud_program, "projection");

  loc_ui_u_time = glGetUniformLocation(hud_program, "u_time");

  loc_ui_font_slot = glGetUniformLocation(hud_program, "ui_font_slot");
  loc_ui_text_color = glGetUniformLocation(hud_program, "ui_text_color");

  glUseProgram(hud_program);
  glUniform1i(loc_ui_font_slot,
              0); // temporary, just assume the font will be bound to slot 0

  // init the pipeline with sensible default settings.
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0 +
                  0); // bind slot zero, do this from the offset zero to
                      // get around having to manually put in the enum.

  return 0;
}

int l_should_close() { return glfwWindowShouldClose(window); }

int l_begin_draw() {
  glClearColor(0.1F, 0.1F, 0.3F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return 0;
}

float u_time = 0.016;

int l_update() {
  glfwPollEvents(); // call this before the input update, this signals the
                    // callbacks.
  if (i_state.act_just_pressed[ACT_SCREENSHOT]) {
    save_screenshot(window, "screen.png");
  }
  glUniform1f(loc_u_time, u_time);
  u_time += delta_time;
  return 0;
}

int l_end_draw() {
  glfwSwapBuffers(window);
  return 0;
}

int l_clean() {
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
