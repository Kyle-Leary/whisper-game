#include <stdio.h>
#include <stdlib.h>

#include "shader.h"
#include "size.h"
#include "util.h"
#include "whisper/hashmap.h"

#define SOURCE_BUF_SZ KB(20)

// if this is const and we change it, we run the risk of segfaulting, since the
// compiler optimizes around the assumption that the string data will not
// change. in other words: only things that are constant should be "const"!
static char source_buffers[1][SOURCE_BUF_SZ] = {0};

WHashMap shader_map = {0};

Shader *make_shader(const char *vs_path, const char *fs_path) {
  Shader *returned_shader = (Shader *)malloc(sizeof(Shader));
  for (int i = 0; i < HASHMAP_SIZE; i++) {
    // the sentinel value of 0 doesn't really work for shader loc ints. set it
    // manually and use -1 instead.
    returned_shader->locs[i].as_int = -1;
  }

  GLint compiled;
  GLchar infoLog[512];
  // use the same buffer for both.

  // Load and compile the vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  read_file_data(vs_path, source_buffers[0], SOURCE_BUF_SZ - 1);

  // we NEED the argument to be const. or else the function call simply won't
  // work. copy the global static pointer into a const member to make this not
  // segfault, THEN compile the const char*.
  const char *source_ptr = source_buffers[0];

  // when a function says that they take in a const char*, take it seriously.
  // const is read-only, read-only is a segment attribute in the elf
  // mem-mapping. if we write to read-only memory... SEGFAULT!!!

  glShaderSource(vertexShader, 1, &source_ptr, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("Vertex shader compilation failed: %s\n", infoLog);
    return 0;
  }

  printf("Compiled vs\n");

  // Load and compile the fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  read_file_data(fs_path, source_buffers[0], SOURCE_BUF_SZ - 1);
  glShaderSource(fragmentShader, 1, &source_ptr, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    printf("Fragment shader compilation failed: %s\n", infoLog);
    return 0;
  }

  printf("Compiled fs\n");

  // Create a shader program
  GLint shaderProgram = glCreateProgram();

  // Attach shaders and link the program
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  GLint status;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
  if (!status) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("Shader program linking failed: %s\n", infoLog);
    return NULL;
  }

  returned_shader->id = shaderProgram;

  // Clean up
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return returned_shader;
}

// caching function over the hashmap.
static GLint get_uniform_loc(Shader *shader, const char *uniform_name) {
  int location = w_hm_get(shader->locs, (char *)uniform_name).as_int;
  if (location == -1) {
    // location not found, ask opengl
    location = glGetUniformLocation(shader->id, uniform_name);
    if (location == -1) {
      // it actually wasn't found properly through the api. print an error.
      fprintf(stderr,
              "ERROR: could not find the uniform with name %s, not found in "
              "cache or OPENGL.\n",
              uniform_name);
    } else {
      // only set it in the hashmap if it's actually a non-sentinel value.
      w_hm_put_direct_value(shader->locs, (char *)uniform_name,
                            (WHashMapValue){.as_int = location});
    }
  }
  return location;
}

// the current program id, good for not doing unnecessary shader switches since
// they need to be USEd in the uniform setter functions.
Shader *curr_program = NULL;

void shader_use(Shader *program) {
  // skip the check if the curr_program is NULL and just set the pointer
  // directly.
  if (curr_program && program->id == curr_program->id)
    return;

  curr_program = program;
  glUseProgram(curr_program->id);
}

void shader_use_name(const char *name) {
  shader_use(w_hm_get(shader_map, name).as_ptr);
}

void shader_set_1f(Shader *shader, const char *uniform_name, float f0) {
  shader_use(shader);
  glUniform1f(get_uniform_loc(shader, uniform_name), f0);
}

void shader_set_2f(Shader *shader, const char *uniform_name, float f0,
                   float f1) {
  shader_use(shader);
  glUniform2f(get_uniform_loc(shader, uniform_name), f0, f1);
}

void shader_set_3f(Shader *shader, const char *uniform_name, float f0, float f1,
                   float f2) {
  shader_use(shader);
  glUniform3f(get_uniform_loc(shader, uniform_name), f0, f1, f2);
}

void shader_set_4f(Shader *shader, const char *uniform_name, float f0, float f1,
                   float f2, float f3) {
  shader_use(shader);
  glUniform4f(get_uniform_loc(shader, uniform_name), f0, f1, f2, f3);
}

void shader_set_1i(Shader *shader, const char *uniform_name, int i0) {
  shader_use(shader);
  glUniform1i(get_uniform_loc(shader, uniform_name), i0);
}

void shader_set_2i(Shader *shader, const char *uniform_name, int i0, int i1) {
  shader_use(shader);
  glUniform2i(get_uniform_loc(shader, uniform_name), i0, i1);
}

void shader_set_3i(Shader *shader, const char *uniform_name, int i0, int i1,
                   int i2) {
  shader_use(shader);
  glUniform3i(get_uniform_loc(shader, uniform_name), i0, i1, i2);
}

void shader_set_4i(Shader *shader, const char *uniform_name, int i0, int i1,
                   int i2, int i3) {
  shader_use(shader);
  glUniform4i(get_uniform_loc(shader, uniform_name), i0, i1, i2, i3);
}

void shader_set_matrix2fv(Shader *shader, const char *uniform_name,
                          const float *value) {
  shader_use(shader);
  glUniformMatrix2fv(get_uniform_loc(shader, uniform_name), 1, GL_FALSE, value);
}

void shader_set_matrix3fv(Shader *shader, const char *uniform_name,
                          const float *value) {
  shader_use(shader);
  glUniformMatrix3fv(get_uniform_loc(shader, uniform_name), 1, GL_FALSE, value);
}

void shader_set_matrix4fv(Shader *shader, const char *uniform_name,
                          const float *value) {
  shader_use(shader);
  glUniformMatrix4fv(get_uniform_loc(shader, uniform_name), 1, GL_FALSE, value);
}

void shader_set_block(Shader *shader, const char *uniform_name,
                      const void *value) {
  shader_use(shader);
  glUniformMatrix4fv(get_uniform_loc(shader, uniform_name), 1, GL_FALSE, value);
}
