#include "console.h"
#include "backends/graphics/shader.h"
#include "backends/graphics_api.h"
#include "backends/ogl_includes.h"
#include "cglm/affine-pre.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "meshing/font.h"
#include "path.h"
#include "whisper/queue.h"
#include <GL/gl.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

Font *console_font;
Shader *console_program;
mat4 c_projection, c_model;

// in glfw
#define NUM_KEYS 348

typedef struct ConsoleInput {
  WQueue char_queue; // use a queuing structure to handle the key events.

  int last_input_state[NUM_KEYS]; // list of either 0 or 1 for each key.
} ConsoleInput;

#define MAX_CONSOLE_INPUT_EVENTS 32

static ConsoleInput c_input = {0};

typedef struct ConsoleRender {
  uint vao;
  uint n_idx;
  vec2 position;
} ConsoleRender;

#define NUM_CONSOLE_RENDERS 10

// either vao or n_idx being zero should indicate an invalid ConsoleRender.
static ConsoleRender renders[NUM_CONSOLE_RENDERS] = {0};

#define LINE_BUF_SZ 1024

typedef struct ConsoleLine {
  ConsoleRender *cr;
  char buffer[LINE_BUF_SZ];
  uint len;
} ConsoleLine;

// a text entry is just a special use of a ConsoleLine.
static ConsoleLine test_entry = {0};

void console_string_render(ConsoleRender *cr,
                           const char *str) { // init test render.
  uint len = strlen(str);

  int num_verts = len * 4;
  int num_indices = len * 6;

  float positions[num_verts * 2];
  float uvs[num_verts * 2];
  uint indices[num_indices];

  cr->n_idx = num_indices;

  font_mesh_string_raw(console_font, str, len, 0.1, 0.1, positions, uvs,
                       indices);

  { // init test render in opengl.
    glGenVertexArrays(1, &(cr->vao));
    glBindVertexArray(cr->vao);

    GLuint vbo_pos_id;
    glGenBuffers(1, &vbo_pos_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_verts, positions,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);

    GLuint vbo_uv_id;
    glGenBuffers(1, &vbo_uv_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_uv_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_verts, uvs,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (GLvoid *)0);
    glEnableVertexAttribArray(1);

    GLuint ebo_id;
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * num_indices, indices,
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
  }
}

void console_init() {
  { // input structure init
    w_make_queue(&(c_input.char_queue), sizeof(char), MAX_CONSOLE_INPUT_EVENTS);
  }

  console_font =
      font_init(16, 16, textures[g_load_texture(TEXTURE_PATH("ui_font.png"))]);

  // init mats
  {
    glm_ortho(0, 1, 0, 1, -5.0f, 5.0f, c_projection);
    glm_mat4_identity(c_model);
  }

  {
    // init test renders
    console_string_render(&(renders[0]), "hello");
    console_string_render(&(renders[1]), "world");
    renders[0].position[0] = 0.5;
    renders[0].position[1] = 0.5;

    renders[1].position[0] = 0.5;
    renders[1].position[1] = 0.3;

    { // test out the textentry system.
      renders[2].position[0] = 0.5;
      renders[2].position[1] = 0.7;
      test_entry.cr = &(renders[2]);
    }
  }

  // init shader w/ uniforms
  {
    console_program =
        make_shader(SHADER_PATH("console.vs"), SHADER_PATH("console.fs"));
    shader_set_1i(console_program, "u_tex_sampler", 0); // 0th slot
    // fix the projection at shader-creation time.
    shader_set_matrix4fv(console_program, "u_projection",
                         (float *)c_projection);
  }
}

void console_handle_input(int key, int scancode, int action, int mods) {
#define IS_SHIFT (mods & GLFW_MOD_SHIFT)
#define IS_CTRL (mods & GLFW_MOD_CTRL)

  if (key < 255 && key > 0) {
    // we're looking at an action representing an ASCII value.
    switch (action) {
    case GLFW_PRESS:
      if (!c_input.last_input_state[key]) {
        // copy the actual character value itself into the queue slot.
        char final_key = IS_SHIFT ? key : tolower(key);
        w_enqueue(&(c_input.char_queue), &final_key);
      }
      break;
    case GLFW_RELEASE:
      if (c_input.last_input_state[key]) {
        // we've just released this key.
      }
      break;
    default:
      break;
    }
  }

  c_input.last_input_state[key] = 1;

#undef IS_SHIFT
#undef IS_CTRL
}

void console_update() {
  {
    WQueue *w = &(c_input.char_queue);
    WQueueSaveState s;
    w_queue_save_state(w, &s);

    bool did_change;

    for (;;) {
      if (test_entry.len >= LINE_BUF_SZ) {
        fprintf(stderr, "ERROR: no more room in textentry character buffer.\n");
        break;
      }

      char *input_ch_ptr = (char *)w_dequeue(w);
      if (!input_ch_ptr)
        break;

      char input_ch = *input_ch_ptr;
      test_entry.buffer[test_entry.len] = input_ch;
      test_entry.len++;

      did_change++;
    }

    if (did_change) {
      console_string_render(test_entry.cr, test_entry.buffer);
    }

    w_queue_load_state(w, &s);
  }

  { // then, we need to just clear the queue.
    WQueue *w = &(c_input.char_queue);
    for (;;) {
      char *input_ch_ptr = (char *)w_dequeue(w);
      if (!input_ch_ptr)
        break;
    }
  }

  { // input update, clear the input array
    memset(&(c_input.last_input_state), 0, sizeof(c_input.last_input_state));
  }
}

void console_draw() {
  g_use_texture(console_font->tex_handle, 0);

  for (int i = 0; i < NUM_CONSOLE_RENDERS; i++) {
    ConsoleRender cr = renders[i]; // we only need a stack copy.
    if (cr.vao == 0)
      continue;

    glm_mat4_identity(c_model);
    glm_translate(c_model, (vec3){cr.position[0], cr.position[1], 0});
    // keep the u_model updated in a loop.
    shader_set_matrix4fv(console_program, "u_model", (float *)c_model);
    glBindVertexArray(cr.vao);
    glDrawElements(GL_TRIANGLES, cr.n_idx, GL_UNSIGNED_INT, 0);
  }
}

void console_clean() {}
