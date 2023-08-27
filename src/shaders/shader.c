#include <stdio.h>
#include <stdlib.h>

#include "macros.h"
#include "shader.h"
#include "size.h"
#include "util.h"
#include "whisper/hashmap.h"
#include "whisper/macros.h"

#include "backends/ogl_includes.h"

#define SOURCE_BUF_SZ KB(20)

typedef enum ShaderType {
  SH_VERT,
  SH_FRAG,
  SH_TES_CTRL,
  SH_TES_EVAL,
  SH_GEOM,
  SH_COUNT,
  SH_INVALID,
} ShaderType;

// allocate one buffer for each ShaderType index.
static char source_buffers[SH_COUNT][SOURCE_BUF_SZ] = {0};

void shader_general_setup(Shader *s) {
  // clear the shader location table with dummy values that indicate there's no
  // shader loc there.
  for (int i = 0; i < HASHMAP_SIZE; i++) {
    s->locs[i].as_int = -1;
  }
}

int make_shader(const char *shader_path) {
  // again, repeat this pattern of using the enum variants as readily-available
  // indices for data.
  size_t total_lens[SH_COUNT] = {0};

  { // read out all the different shader types into their proper buffer.
    FILE *file = fopen(shader_path, "r");
    NULL_CHECK(file);

    ShaderType curr_type = SH_INVALID;

    static const char *vert_str = "#vertex";
    static size_t vert_str_len = 7;
    static const char *frag_str = "#fragment";
    static size_t frag_str_len = 9;
    static const char *tes_ctrl_str = "#tes_ctrl";
    static size_t tes_ctrl_str_len = 9;
    static const char *tes_eval_str = "#tes_eval";
    static size_t tes_eval_str_len = 9;
    static const char *geom_str = "#geometry";
    static size_t geom_str_len = 9;

    char line[512];

    while (fgets(line, sizeof(line), file)) {
      // null terminate the old type's buffer, then setup the new type for
      // writing.
#define SWITCH(variant)                                                        \
  {                                                                            \
    if (curr_type != SH_INVALID)                                               \
      source_buffers[curr_type][total_lens[curr_type]] = '\0';                 \
    curr_type = variant;                                                       \
    continue;                                                                  \
  }

      if (strncmp(line, vert_str, vert_str_len) == 0) {
        SWITCH(SH_VERT);
      } else if (strncmp(line, frag_str, frag_str_len) == 0) {
        SWITCH(SH_FRAG);
      } else if (strncmp(line, tes_ctrl_str, tes_ctrl_str_len) == 0) {
        SWITCH(SH_TES_CTRL);
      } else if (strncmp(line, tes_eval_str, tes_eval_str_len) == 0) {
        SWITCH(SH_TES_EVAL);
      } else if (strncmp(line, geom_str, geom_str_len) == 0) {
        SWITCH(SH_GEOM);
      }

#undef SWITCH

#define CASE(variant)                                                          \
  case variant: {                                                              \
    size_t line_len = strlen(line);                                            \
    strcpy(source_buffers[variant] + total_lens[variant], line);               \
    total_lens[variant] += line_len;                                           \
  } break;

      switch (curr_type) {
        CASE(SH_VERT);
        CASE(SH_FRAG);
        CASE(SH_TES_CTRL);
        CASE(SH_TES_EVAL);
        CASE(SH_GEOM);
      default:
        break;
      }
    }

#undef CASE

    fclose(file);
  }

  GLint compiled;
  GLchar infoLog[512];
  // use the same buffer for both.

  // we NEED the argument to be const. or else the function call simply won't
  // work. copy the global static pointer into a const member to make this not
  // segfault, THEN compile the const char*.
  const char *vert_ptr = source_buffers[SH_VERT];
  const char *frag_ptr = source_buffers[SH_FRAG];
  const char *tes_ctrl_ptr = source_buffers[SH_TES_CTRL];
  const char *tes_eval_ptr = source_buffers[SH_TES_EVAL];
  const char *geom_ptr = source_buffers[SH_GEOM];

  // when a function says that they take in a const char*, take it seriously.
  // const is read-only, read-only is a segment attribute in the elf
  // mem-mapping. if we write to read-only memory... SEGFAULT!!!

  // Create a shader program
  int shaderProgram = glCreateProgram();

  { // compile into the shader program. keep track of how many non-empty buffers
    // we compile, and check to see if nothing was compiled.
    int num_compiled = 0;

#define COMPILE(variant, sh_type, sh_ptr)                                      \
  do {                                                                         \
    if (total_lens[variant] == 0)                                              \
      break;                                                                   \
    num_compiled++;                                                            \
    GLuint into = glCreateShader(sh_type);                                     \
    glShaderSource(into, 1, &sh_ptr, NULL);                                    \
    glCompileShader(into);                                                     \
    glGetShaderiv(into, GL_COMPILE_STATUS, &compiled);                         \
    if (!compiled) {                                                           \
      glGetShaderInfoLog(into, 512, NULL, infoLog);                            \
      fprintf(stderr, #variant " shader compilation failed: %s\n", infoLog);   \
      exit(1);                                                                 \
    }                                                                          \
    glAttachShader(shaderProgram, into);                                       \
    glDeleteShader(into);                                                      \
  } while (0)

    COMPILE(SH_VERT, GL_VERTEX_SHADER, vert_ptr);
    COMPILE(SH_FRAG, GL_FRAGMENT_SHADER, frag_ptr);
    COMPILE(SH_TES_CTRL, GL_TESS_CONTROL_SHADER, tes_ctrl_ptr);
    COMPILE(SH_TES_EVAL, GL_TESS_EVALUATION_SHADER, tes_eval_ptr);
    COMPILE(SH_GEOM, GL_GEOMETRY_SHADER, frag_ptr);

#undef COMPILE

    if (num_compiled == 0) {
      fprintf(stderr,
              "ERROR: Nothing was compiled during the shader compilation of "
              "%s! [%s, %s]\n",
              shader_path, __PRETTY_FUNCTION__, __FILE__);
      exit(1);
    }
  }

  glLinkProgram(shaderProgram);

  GLint status;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
  if (!status) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("Shader program linking failed: %s\n", infoLog);
    return NULL;
  }

  return shaderProgram;
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

// the current program id, good for not doing unnecessary shader switches
// since they need to be USEd in the uniform setter functions.
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
