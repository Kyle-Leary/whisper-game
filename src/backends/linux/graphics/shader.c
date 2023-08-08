#include <stdio.h>
#include <stdlib.h>

#include "shader.h"
#include "util.h"

#define SOURCE_BUF_SZ 2048

// if this is const and we change it, we run the risk of segfaulting, since the
// compiler optimizes around the assumption that the string data will not
// change. in other words: only things that are constant should be "const"!
static char source_buffers[1][SOURCE_BUF_SZ] = {0};

GLuint make_shader(const char *vs_path, const char *fs_path) {
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
  GLuint shaderProgram = glCreateProgram();

  // Attach shaders and link the program
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &compiled);
  if (!compiled) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("Shader program linking failed: %s\n", infoLog);
    return 0;
  }

  // Clean up
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}
