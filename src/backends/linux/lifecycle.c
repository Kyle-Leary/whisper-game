#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "backends/lifecycle_api.h"

#include "backends/linux/graphics/shader.h"
#include "global.h"
#include "graphics/linux_graphics_globals.h"
#include "main.h"
#include "ogl_includes.h"
#include "path.h"
#include "whisper/hashmap.h"
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// define the externs in ogl_includes.h
GLFWwindow *window = NULL;

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

static GLuint light_data_ubo = 0;

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
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

#define INSERT(name)                                                           \
  w_hm_put_direct_value(shader_map, #name,                                     \
                        (WHashMapValue){.as_ptr = name##_program})

  // the "default" shader for normal 3d scenes
  Shader *basic_program =
      make_shader(SHADER_PATH("basic.vs"), SHADER_PATH("basic.fs"));
  shader_set_1i(basic_program, "main_slot", 0);
  INSERT(basic);

  // the default for the hud rendering, eg text stuff. we don't really want to
  // render the hud in perspective.
  Shader *hud_program =
      make_shader(SHADER_PATH("hud.vs"), SHADER_PATH("hud.fs"));
  shader_set_1i(hud_program, "ui_font_slot", 0);
  INSERT(hud);

  Shader *gouraud_program =
      make_shader(SHADER_PATH("gouraud.vs"), SHADER_PATH("gouraud.fs"));
  INSERT(gouraud);

  Shader *pbr_gouraud_program =
      make_shader(SHADER_PATH("pbr_gouraud.vs"), SHADER_PATH("pbr_gouraud.fs"));
  INSERT(pbr_gouraud);

  Shader *solid_program =
      make_shader(SHADER_PATH("solid.vs"), SHADER_PATH("solid.fs"));
  INSERT(solid);

  Shader *skybox_program =
      make_shader(SHADER_PATH("skybox.vs"), SHADER_PATH("skybox.fs"));
  INSERT(skybox);

  { // SET UP UBOS
    unsigned int blockIndex =
        glGetUniformBlockIndex(gouraud_program->id, "LightData");

    glUniformBlockBinding(gouraud_program->id, blockIndex, 0);

    glGenBuffers(1, &light_data_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, light_data_ubo);
    // we're going to change the ubo frequently with subdata calls, so make this
    // dynamic.
    glBufferData(GL_UNIFORM_BUFFER, sizeof(g_light_data), NULL,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, light_data_ubo, 0,
                      sizeof(g_light_data));
  }

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
  { // write the light data to the active shader through a UBO.
    // Step 4: Bind the UBO to the same binding point
    glBindBuffer(GL_UNIFORM_BUFFER, light_data_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(g_light_data), &g_light_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

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

  shader_set_1f(w_hm_get(shader_map, "basic").as_ptr, "u_time", u_time);

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
