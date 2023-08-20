#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "backends/lifecycle_api.h"

#include "backends/graphics/shader.h"
#include "cglm/mat4.h"
#include "global.h"
#include "graphics/graphics_globals.h"
#include "main.h"
#include "ogl_includes.h"
#include "path.h"
#include "util.h"
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

// which ubo block slots should these structures occupy?
#define LIGHT_BLOCK 0
#define MATRIX_BLOCK 1

static GLuint light_data_ubo = 0;
static GLuint matrix_data_ubo = 0;
GLuint bone_data_ubo = 0;
GLuint material_data_ubo = 0;

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

#define INSERT(name)                                                           \
  w_hm_put_direct_value(shader_map, #name,                                     \
                        (WHashMapValue){.as_ptr = name##_program})

  // the "default" shader for normal 3d scenes
  Shader *basic_program =
      make_shader(SHADER_PATH("basic.vs"), SHADER_PATH("basic.fs"));
  shader_set_1i(basic_program, "main_slot", 0);
  INSERT(basic);

  // 0 - 1, 0 - 1 orthographic projection to screenspace for the UI. have the UI
  // coordinates not scale with the screensize, so that we can resize the
  // window?
  glm_ortho(0, 1, 0, 1, -5.0f, 5.0f, m_ui_projection);

  // for now, i think we can just get away with initting the projection for the
  // UI once in the start, and immediately binding the projection data to each
  // UI shader.

  // flat rendering program for the hud.
  Shader *hud_program =
      make_shader(SHADER_PATH("hud.vs"), SHADER_PATH("hud.fs"));
  shader_set_1i(hud_program, "tex_sampler", 0);
  shader_set_matrix4fv(hud_program, "projection", (float *)m_ui_projection);
  INSERT(hud);

  Shader *hud_text_program =
      make_shader(SHADER_PATH("hud_text.vs"), SHADER_PATH("hud_text.fs"));
  shader_set_1i(hud_text_program, "text_font_slot", FONT_TEX_SLOT);
  shader_set_matrix4fv(hud_text_program, "projection",
                       (float *)m_ui_projection);
  shader_set_3f(hud_text_program, "text_base_color", 0.1, 0.5, 0.5);
  INSERT(hud_text);

  Shader *hud_text_wavy_program = make_shader(SHADER_PATH("hud_text_wavy.vs"),
                                              SHADER_PATH("hud_text_wavy.fs"));
  shader_set_1i(hud_text_wavy_program, "text_font_slot", FONT_TEX_SLOT);
  shader_set_matrix4fv(hud_text_wavy_program, "projection",
                       (float *)m_ui_projection);
  shader_set_3f(hud_text_wavy_program, "text_base_color", 0.1, 0.5, 0.5);
  INSERT(hud_text_wavy);

  Shader *gouraud_program =
      make_shader(SHADER_PATH("gouraud.vs"), SHADER_PATH("gouraud.fs"));
  INSERT(gouraud);

  Shader *pbr_gouraud_program =
      make_shader(SHADER_PATH("pbr_gouraud.vs"), SHADER_PATH("pbr_gouraud.fs"));
  INSERT(pbr_gouraud);

  Shader *solid_program =
      make_shader(SHADER_PATH("solid.vs"), SHADER_PATH("solid.fs"));
  INSERT(solid);

  Shader *text_3d_program =
      make_shader(SHADER_PATH("text_3d.vs"), SHADER_PATH("text_3d.fs"));
  shader_set_1i(text_3d_program, "text_font_slot", FONT_TEX_SLOT);
  shader_set_3f(text_3d_program, "text_base_color", 0.1, 0.5, 0.5);
  INSERT(text_3d);

  Shader *skybox_program =
      make_shader(SHADER_PATH("skybox.vs"), SHADER_PATH("skybox.fs"));
  INSERT(skybox);

  // model renderer
  Shader *model_program =
      make_shader(SHADER_PATH("model.vs"), SHADER_PATH("model.fs"));
  shader_set_1i(model_program, "tex_sampler",
                0); // the character texture should be stored in the 0th slot
                    // by default.
  INSERT(model);

  { // SET UP UBOS
#define APPLY_BINDINGS(BIND_FUNC, PROGRAM)                                     \
  printf("applying to " #PROGRAM "\n");                                        \
  BIND_FUNC("LightData", LIGHT_BLOCK, PROGRAM);                                \
  BIND_FUNC("ViewProjection", MATRIX_BLOCK, PROGRAM);

#define BIND_PROGRAMS(PROGRAM)                                                 \
  { APPLY_BINDINGS(ID_TO_BLOCK, PROGRAM); }

#define ID_TO_BLOCK(block_name_literal, block_target_index, shader_id)         \
  {                                                                            \
    unsigned int block_index =                                                 \
        glGetUniformBlockIndex(shader_id, block_name_literal);                 \
    printf("found block index %d\n", block_index);                             \
    glUniformBlockBinding(shader_id, block_index, block_target_index);         \
  }

    printf("apply ubo start: \n");

    // BIND_ binds to all the default block locations.
    // if you want to use the block data, you need to bind the shader's internal
    // block with the global UBO slot.
    BIND_PROGRAMS(gouraud_program->id);
    BIND_PROGRAMS(pbr_gouraud_program->id);
    BIND_PROGRAMS(basic_program->id);
    BIND_PROGRAMS(solid_program->id);
    BIND_PROGRAMS(skybox_program->id);
    BIND_PROGRAMS(text_3d_program->id);

    // only the model program is using the BONE_BLOCK uniform.
    BIND_PROGRAMS(model_program->id);
    ID_TO_BLOCK("BoneData", BONE_BLOCK, model_program->id);
    ID_TO_BLOCK("MaterialBlock", MATERIAL_BLOCK, model_program->id);

    GLuint buf[4];
    glGenBuffers(4, buf);
    light_data_ubo = buf[0];
    matrix_data_ubo = buf[1];
    bone_data_ubo = buf[2];
    material_data_ubo = buf[3];

    glBindBuffer(GL_UNIFORM_BUFFER, light_data_ubo);
    // we're going to change the ubo frequently with subdata calls, so make this
    // dynamic.
    glBufferData(GL_UNIFORM_BUFFER, sizeof(g_light_data), NULL,
                 GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, LIGHT_BLOCK, light_data_ubo, 0,
                      sizeof(g_light_data));

    // space for two matrices. this doesn't require padding like the light data.
    glBindBuffer(GL_UNIFORM_BUFFER, matrix_data_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 32, NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, MATRIX_BLOCK, matrix_data_ubo, 0,
                      sizeof(float) * 32);

    glBindBuffer(GL_UNIFORM_BUFFER, bone_data_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(BoneData), NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, BONE_BLOCK, bone_data_ubo, 0,
                      sizeof(BoneData));

    glBindBuffer(GL_UNIFORM_BUFFER, material_data_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialData), NULL,
                 GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, MATERIAL_BLOCK, material_data_ubo, 0,
                      sizeof(MaterialData));
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
  { // update UBOs.
    // these will just change per frame usually, so update them in a loop
    // instead of making a whole new graphics api function just to change them.
    { // light
      glBindBuffer(GL_UNIFORM_BUFFER, light_data_ubo);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(g_light_data),
                      &g_light_data);
    }

    { // matrix
      glBindBuffer(GL_UNIFORM_BUFFER, matrix_data_ubo);
      // view, then the projection.
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, m_view);
      glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16,
                      m_projection);
    }

    // we're not updating bone ubo in a loop, just update that before we draw a
    // model, calling the specified graphics api function.
  }

  glClearColor(0.1F, 0.1F, 0.3F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return 0;
}

int l_update() {
  glfwPollEvents(); // call this before the input update, this signals the
                    // callbacks.
  if (i_state.act_just_pressed[ACT_SCREENSHOT]) {
    save_screenshot(window, "screen.png");
  }

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
