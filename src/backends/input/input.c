#include "backends/input_api.h"

// share the opengl includes file with the rest of the opengl/glfw linux
// backend. we're using the same backend stuff for input AND graphics.
#include "../graphics/graphics_globals.h"
#include "../ogl_includes.h"

#include "console/console.h"
#include "defines.h"
#include "global.h"
#include "helper_math.h"
#include "main.h"
#include "size.h"
#include <GLFW/glfw3.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// don't read the whole file at once, stream it into the global inputstate using
// this module.
typedef struct Replay {
  char file_path[FILEPATH_BUF_SZ];
  FILE *file_stream;
  int frame; // which frame of the input are we ticking through right now?
  bool does_loop;
} Replay;

// ideally, the replay can be entirely internal?
static Replay i_curr_replay = {0};
static bool i_is_playing = false;

#define NUM_KEYS 348

static int action_map[NUM_KEYS] = {
    [GLFW_KEY_W] = ACT_UP,
    [GLFW_KEY_A] = ACT_LEFT,
    [GLFW_KEY_S] = ACT_DOWN,
    [GLFW_KEY_D] = ACT_RIGHT,
    [GLFW_KEY_SPACE] = ACT_JUMP,
    [GLFW_MOUSE_BUTTON_LEFT] = ACT_HUD_INTERACT,
    [GLFW_MOUSE_BUTTON_RIGHT] = ACT_IS_SWINGING,
    [GLFW_KEY_9] = ACT_SCREENSHOT,

    [GLFW_KEY_Y] = ACT_TOGGLE_DEBUG_DRAW,
    [GLFW_KEY_C] = ACT_TOGGLE_DEBUG_CONSOLE,

    [GLFW_KEY_EQUAL] = ACT_INCREASE_TIMESCALE,
    [GLFW_KEY_MINUS] = ACT_DECREASE_TIMESCALE,

    [GLFW_KEY_RIGHT] = ACT_CAMERA_CW,
    [GLFW_KEY_LEFT] = ACT_CAMERA_CCW,

    [GLFW_KEY_UP] = ACT_CAMERA_RAISE,
    [GLFW_KEY_DOWN] = ACT_CAMERA_LOWER,
}; // the surjective mapping from "key-like" inputs to
   // ActionTypes. hacky as shit.

InputState i_state = {0};

#define PRESSED input_state.pressed

float fwd_speed = 1.0F;
float rot_speed = 0.05F;

// input handling across the mouse and keyboard input is fully generic. just
// pass in the action and enum, and operate on those, since LEFT_MOUSE and KEY_W
// are in the same enum space.
static void handle_generic_input(int key, int action) {
  // we can't zero-alloc everything automatically (while still setting the
  // inputs that i WANT set), and i don't want to pass through the array at
  // runtime because that's slow as fuck, so we'll just assume that any return
  // out of bounds of the actions array is a bad mapping. this is an incorrect
  // assumption, since a random byte of memory could easily point to an inbounds
  // number for the enum, but this'll do.
  int action_type = action_map[key];
  if (action_type > ACT_COUNT) // comparing against enum, it won't be less than
                               // 0 so we're fine?
    return;

  switch (action) {
  case GLFW_PRESS:                              // When the key is pressed
    if (!i_state.last_act_state[action_type]) { // If the button was not
                                                // previously held
      i_state.act_just_pressed[action_type] = 1;
    }
    i_state.act_held[action_type] = 1;
    break;
  case GLFW_RELEASE: // When the key is released
    if (i_state
            .last_act_state[action_type]) { // If the button was previously held
      i_state.act_just_released[action_type] = 1;
    }
    i_state.act_held[action_type] = 0;
    break;
  default:
    break;
  }

  i_state.last_act_state[action_type] =
      i_state.act_held[action_type]; // Remember the current state for the next
                                     // callback
}

static void input_key_callback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  console_handle_input(key, scancode, action, mods);

  if (i_is_playing) {
  } else {
    InputState *state = &i_state;

    // extremely generic input actions.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    rot_dx = 0;
    forward_dx = 0;

    handle_generic_input(key, action);
  }
}

static void input_mouse_button_callback(GLFWwindow *window, int button,
                                        int action, int mods) {
  if (i_is_playing) {
  } else {
    handle_generic_input(button,
                         action); // the "button" is actually just a key.
  }
}

static void input_cursor_position_callback(GLFWwindow *window, double xpos,
                                           double ypos) {
  if (i_is_playing) {
  } else {
    memcpy(i_state.prev_pointer, i_state.pointer, sizeof(float) * 2);

    if (delta_time > 0) { // Avoid division by zero
      // Compute speed (difference in position / difference in time)
      i_state.pointer_velocity[0] =
          (xpos / WIN_W - i_state.prev_pointer[0]) / delta_time;
      i_state.pointer_velocity[1] =
          (1 - ypos / WIN_H - i_state.prev_pointer[1]) / delta_time;
    } else {
      // No time has passed, velocity is 0
      i_state.pointer_velocity[0] = 0;
      i_state.pointer_velocity[1] = 0;
    }

    // this is the most common case, reference the pointer position wrt the ui
    // screenspace coords with bottom to top y and a percentage rather than a
    // pixel.
    i_state.pointer[0] = xpos / WIN_W;
    i_state.pointer[1] = 1 - (ypos / WIN_H);
  }
}

// register the callbacks locally. this way, even in the plugin backend, we can
// STILL use the callbacks natively.
void i_init() {
  // using the global window object shared through the ogl_includes.h header.
  glfwSetKeyCallback(window, input_key_callback);
  glfwSetMouseButtonCallback(window, input_mouse_button_callback);
  glfwSetCursorPosCallback(window, input_cursor_position_callback);
}

void i_update() {
  if (i_is_playing) {
    if (feof(i_curr_replay.file_stream)) {
      // we've reached the end of a file, either stop playing or loop around.
      if (i_curr_replay.does_loop) {
        // then loop.
        fseek(i_curr_replay.file_stream, 0, SEEK_SET);
        i_curr_replay.frame = 0;
      } else {
        // otherwise, stop the playback.
        i_stop_replay();
      }
    } else {
      // or, this is a normal case and we just keep reading from the input file.

      // read directly into the global input state.
      fread(&i_state, sizeof(InputState), 1, i_curr_replay.file_stream);

      // keep this helper up to date.
      i_curr_replay.frame++;
    }
  } else {
    // Reset the just_pressed and just_released arrays
    memset(i_state.act_just_pressed, 0, sizeof(i_state.act_just_pressed));
    memset(i_state.act_just_released, 0, sizeof(i_state.act_just_released));
  }

  // generally handle the pointer and other internal input model updates.
  glm_vec2_scale(
      i_state.pointer_velocity, 0.2,
      i_state
          .pointer_velocity); // return the pointer_velocity variable to zero.
}

void i_clean() {}

void i_start_replay(const char *inputs_path, bool does_loop) {
  i_curr_replay.file_stream = fopen(inputs_path, "rb");
  if (i_curr_replay.file_stream == NULL) {
    fprintf(stderr, "Could not open the input_state serialized file %s: %s\n",
            inputs_path, strerror(errno));
    // don't even try to init the rest of the replay start process.
    return;
  }

  i_is_playing = true;
  i_curr_replay.frame = 0;
  i_curr_replay.does_loop = does_loop;
  strncpy(i_curr_replay.file_path, inputs_path, FILEPATH_BUF_SZ);
}

void i_toggle_replay() {
  if (i_is_playing) {
    i_pause_replay();
  } else {
    i_unpause_replay();
  }
}

void i_pause_replay() { i_is_playing = false; }
void i_unpause_replay() { i_is_playing = true; }

void i_stop_replay() {
  // basically free the internal bits of the replay structure.
  i_is_playing = false;
  if (i_curr_replay.file_stream != NULL) {
    fclose(i_curr_replay.file_stream);
  }

  i_curr_replay.frame = 0;

  // worlds laziest null terminator
  memset(i_curr_replay.file_path, 0, FILEPATH_BUF_SZ);
  strcpy(i_curr_replay.file_path,
         "No current InputState file being played back.\n");
}
