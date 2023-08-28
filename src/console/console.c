#include "console.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "console/commands.h"
#include "helper_math.h"
#include "meshing/font.h"
#include "path.h"
#include "shaders/shader.h"
#include "shaders/shader_binding.h"
#include "shaders/shader_instances.h"
#include "whisper/queue.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "ogl_includes.h"

static bool has_control = false;

void toggle_console() { has_control = !has_control; }

static Font *console_font;
static Shader *console_program;
static mat4 c_projection, c_model;

// in glfw
#define NUM_KEYS 348

typedef struct ConsoleInput {
  WQueue char_queue; // this is a list of shorts. we're using the ASCII range
                     // for characters, and the rest for special characters.

  int last_input_state[NUM_KEYS]; // list of either 0 or 1 for each key.
} ConsoleInput;

#define MAX_CONSOLE_INPUT_EVENTS 32

static ConsoleInput c_input = {0};

typedef struct ConsoleRender {
  uint vao;
  uint n_idx;
} ConsoleRender;

#define NUM_LINES 10
// every line needs its own console render slot.
#define NUM_CONSOLE_RENDERS NUM_LINES

#define RERENDER_LINE(slot)                                                    \
  console_string_render(&(c_graphics.renders[slot]),                           \
                        c_graphics.lines[slot].buffer);

typedef struct ConsoleLine {
  char buffer[LINE_BUF_SZ];
  uint len;
} ConsoleLine;

typedef struct ConsoleGraphics {
  // either vao or n_idx being zero should indicate an invalid ConsoleRender.
  ConsoleRender renders[NUM_CONSOLE_RENDERS];
  ConsoleLine lines[NUM_LINES];
  float line_height;
  char last_command[LINE_BUF_SZ];
  int last_command_sz;
} ConsoleGraphics;

// a text entry is just a special use of a ConsoleLine.
static ConsoleGraphics c_graphics = {0};

#define CONSOLE_TEXT_ENTRY_LINE_SLOT (0)
#define CONSOLE_TEXT_ENTRY (c_graphics.lines[CONSOLE_TEXT_ENTRY_LINE_SLOT])

void console_string_render(ConsoleRender *cr,
                           const char *str) { // init test render.
  uint len = strlen(str);

  int num_verts = len * 4;
  int num_indices = len * 6;

  float positions[num_verts * 2];
  float uvs[num_verts * 2];
  uint indices[num_indices];

  cr->n_idx = num_indices;

  font_mesh_string_raw(console_font, str, len, 0.01, c_graphics.line_height,
                       positions, uvs, indices);

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
  console_program = get_shader("console");
  { // default settings
    c_graphics.line_height = 0.04;
  }

  { // input structure init
    w_make_queue(&(c_input.char_queue), sizeof(short),
                 MAX_CONSOLE_INPUT_EVENTS);
  }

  console_font =
      font_init(16, 16, textures[g_load_texture(TEXTURE_PATH("ui_font.png"))]);

  // init mats
  {
    float fov = glm_rad(45.0f);
    float aspect = 1.0f;
    float near = 0.1f;
    float far = 5.0f;

    glm_perspective(fov, aspect, near, far, c_projection);

    mat4 private_view;
    vec3 camera_pos = {0.5, 0.8, 0}; // Camera at the origin
    vec3 point_pos = {0.5, 0.5, -1}; // The point you want to center
    vec3 up = {0, 1, 0};             // The 'up' direction

    glm_lookat(camera_pos, point_pos, up, private_view);
    glm_mat4_mul(c_projection, private_view, c_projection);

    shader_bind(console_program);
    shader_set_matrix4fv(console_program, "u_projection",
                         (float *)c_projection);

    glm_mat4_identity(c_model);
  }
}

void console_handle_input(int key, int scancode, int action, int mods) {
  if (!has_control)
    return;

#define IS_SHIFT (mods & GLFW_MOD_SHIFT)
#define IS_CTRL (mods & GLFW_MOD_CTRL)

  switch (action) {
  case GLFW_PRESS:
    if (!c_input.last_input_state[key]) {
      if (key < 255 && key > 0) {
        // we're looking at an action representing an ASCII value.
        // copy the actual character value itself into the queue slot.
        short final_key = IS_SHIFT ? key : tolower(key);
        w_enqueue(&(c_input.char_queue), &final_key);
      } else {
        // only apply the tolower to chars. these are special keys, and are
        // copied in as-is.
        w_enqueue(&(c_input.char_queue), &key);
      }
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

  c_input.last_input_state[key] = 1;

#undef IS_SHIFT
#undef IS_CTRL
}

void console_newline() {
  for (int i = NUM_LINES - 2; i >= 0; i--) {
    memcpy(&(c_graphics.lines[i + 1]), &(c_graphics.lines[i]),
           sizeof(ConsoleLine));
    memset(&(c_graphics.lines[i]), 0, sizeof(ConsoleLine));
    RERENDER_LINE(i);
    RERENDER_LINE(i + 1);
  }
}

void console_print(char *text, int len) {
  char *starting_point = CONSOLE_TEXT_ENTRY.buffer + CONSOLE_TEXT_ENTRY.len;
  int bytes_remaining = (CONSOLE_TEXT_ENTRY.buffer + LINE_BUF_SZ) -
                        (CONSOLE_TEXT_ENTRY.buffer + CONSOLE_TEXT_ENTRY.len);
  int safe_range = MIN(bytes_remaining, len);

  int i = 0;
  int buffer_index = CONSOLE_TEXT_ENTRY.len;
  while (text[i] != '\0') {
#define PEEK() (text[i + 1])
#define PUT_CH(ch)                                                             \
  { CONSOLE_TEXT_ENTRY.buffer[buffer_index] = ch; }
#define NEWLINE()                                                              \
  {                                                                            \
    console_newline();                                                         \
    buffer_index = 0;                                                          \
  }

    char ch = text[i];

    // if we've run out of buffer space or hit a newline, flush the line.
    if (ch == '\\') {
      switch (PEEK()) {
      case 'n': {
        // escaped newline
        NEWLINE();
        i++;
      } break;
      default: {
        PUT_CH(ch);
      } break;
      }
    } else if (ch == '\n' || buffer_index >= safe_range) {
      NEWLINE();
    } else {
      PUT_CH(ch);
    }

    i++;
    buffer_index++;

#undef PEEK
#undef PUT_CH
#undef NEWLINE
  }
}

void console_println(char *line_text, int len) {
  console_print(line_text, len);
  console_newline();
}

void console_printf(const char *format, ...) {
  char temp_buf[LINE_BUF_SZ];
  va_list args;
  va_start(args, format);

  int formatted_len = vsnprintf(temp_buf, LINE_BUF_SZ, format, args);
  if (formatted_len >= LINE_BUF_SZ) {
    formatted_len = LINE_BUF_SZ - 1;
  }
  temp_buf[formatted_len] = '\0';

  va_end(args);
  console_print(temp_buf, formatted_len);
}

static void console_submit() {
  memset(
      c_graphics.last_command, 0,
      LINE_BUF_SZ); // make sure there's no dangling stuff on the last command.
  memcpy(c_graphics.last_command, CONSOLE_TEXT_ENTRY.buffer, LINE_BUF_SZ);
  c_graphics.last_command_sz = CONSOLE_TEXT_ENTRY.len;

  console_newline();
  CommandResponse r = {0};
  // the command runner can handle its own printing through the exposed console
  // api.
  command_run(&r, c_graphics.last_command, c_graphics.last_command_sz);
}

// with the up key, get the last command you submitted.
static void console_history_up() {
  memcpy(CONSOLE_TEXT_ENTRY.buffer, c_graphics.last_command, LINE_BUF_SZ);
  CONSOLE_TEXT_ENTRY.len = c_graphics.last_command_sz;
  RERENDER_LINE(0);
}

// be lazy for now.
static void console_history_down() {
  memset(CONSOLE_TEXT_ENTRY.buffer, 0, LINE_BUF_SZ);
  RERENDER_LINE(0);
}

void console_update() {
  if (!has_control)
    return;

  { // update the text entry line with the new input.
    WQueue *w = &(c_input.char_queue);
    WQueueSaveState s;
    w_queue_save_state(w, &s);

    bool did_change;

    for (;;) {
      if (CONSOLE_TEXT_ENTRY.len >= LINE_BUF_SZ) {
        fprintf(stderr, "ERROR: no more room in textentry character buffer.\n");
        break;
      }

      short *input_ch_ptr = (short *)w_dequeue(w);
      if (!input_ch_ptr)
        break;

      short input_ch = *input_ch_ptr;
      if (input_ch < 256) {
        // it's text and we put it in.
        CONSOLE_TEXT_ENTRY.buffer[CONSOLE_TEXT_ENTRY.len] = input_ch;
        CONSOLE_TEXT_ENTRY.len++;
        did_change++;
      } else {
        switch (input_ch) {
        case GLFW_KEY_BACKSPACE: {
          if (CONSOLE_TEXT_ENTRY.len > 0) {
            CONSOLE_TEXT_ENTRY.len--;
            CONSOLE_TEXT_ENTRY.buffer[CONSOLE_TEXT_ENTRY.len] = '\0';
            did_change++;
          } // else, don't backspace since we're out of text.
        } break;
        }
      }
    }

    if (did_change) {
      RERENDER_LINE(0);
    }

    w_queue_load_state(w, &s);
  }

  {
    WQueue *w = &(c_input.char_queue);
    WQueueSaveState s;
    w_queue_save_state(w, &s);

    for (;;) {
      short *input_ch_ptr = (short *)w_dequeue(w);
      if (!input_ch_ptr)
        break;

      short input_ch = *input_ch_ptr;
      if (input_ch > 255) {
        if (input_ch == GLFW_KEY_ENTER) {
          // copy the line entry up one line, then clear the 0th line.
          console_submit();
        } else if (input_ch == GLFW_KEY_UP) {
          console_history_up();
        } else if (input_ch == GLFW_KEY_DOWN) {
          console_history_down();
        }
      }
    }

    w_queue_load_state(w, &s);
  }

  { // then, we need to just clear the queue.
    WQueue *w = &(c_input.char_queue);
    for (;;) {
      void *ptr = (void *)w_dequeue(w);
      if (!ptr)
        break;
    }
  }

  { // input update, clear the input array
    memset(&(c_input.last_input_state), 0, sizeof(c_input.last_input_state));
  }
}

void console_draw() {
  if (!has_control)
    return;

  // batch all the console renders with the same texture.
  g_use_texture(console_font->tex_handle, 0);

  for (int i = 0; i < NUM_CONSOLE_RENDERS; i++) {
    ConsoleRender cr = c_graphics.renders[i]; // we only need a stack copy.
    if (cr.vao == 0)
      continue;

    float x = 0.5;
    float y = c_graphics.line_height * i + 0.5;

    glm_mat4_identity(c_model);
    glm_translate(c_model, (vec3){x, y, 0});
    // keep the u_model updated in a loop.
    shader_set_matrix4fv(console_program, "u_model", (float *)c_model);
    glBindVertexArray(cr.vao);
    glDrawElements(GL_TRIANGLES, cr.n_idx, GL_UNSIGNED_INT, 0);
  }
}

// TODO: how to keep track of and properly free the VAOs we're making?
void console_clean() {}
