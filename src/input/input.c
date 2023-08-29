#include "input.h"
#include "path.h"

#include "macros.h"

#include "console/console.h"
#include "defines.h"
#include "global.h"
#include "helper_math.h"
#include "main.h"
#include "ogl_includes.h"
#include "size.h"

#include "window.h"

#include <GLFW/glfw3.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NUM_KEYS 348

InputState i_state = {0};

typedef enum GLFWInputType {
  INPUT_TYPE_KEYBOARD,
  INPUT_TYPE_MOUSE,
  INPUT_TYPE_COUNT,
} GLFWInputType;

static Input glfw_map_into_input(int key, GLFWInputType type) {
  switch (type) {
  case INPUT_TYPE_KEYBOARD: {
    // this is ID-mapped with glfw inputs.
    return key;
  } break;
  case INPUT_TYPE_MOUSE: {
    return key + MOUSE_LEFT; // map at an offset.
  } break;
  default: {
    ERROR_NO_ARGS("passed invalid GLFWInputType.");
  } break;
  }
}

static void handle_generic_input(int key, int action, int mods,
                                 GLFWInputType type) {
  ActionType input_action =
      i_state.action_mapping[mods][glfw_map_into_input(key, type)];

  switch (action) {
  case GLFW_PRESS:
    if (!i_state.act_held[input_action]) {
      i_state.act_just_pressed[input_action] = 1;
    }
    i_state.act_held[input_action] = 1;
    break;
  case GLFW_RELEASE:
    if (i_state.act_held[input_action]) {
      i_state.act_just_released[input_action] = 1;
    }
    i_state.act_held[input_action] = 0;
    break;
  default:
    break;
  }
}

static void input_key_callback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  console_handle_input(key, scancode, action, mods);

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  handle_generic_input(key, action, mods, INPUT_TYPE_KEYBOARD);
}

static void input_mouse_button_callback(GLFWwindow *window, int button,
                                        int action, int mods) {
  handle_generic_input(button, action, mods, INPUT_TYPE_MOUSE);
}

static void input_cursor_position_callback(GLFWwindow *window, double xpos,
                                           double ypos) {
  memcpy(i_state.prev_pointer, i_state.pointer, sizeof(float) * 2);

  if (delta_time > 0) {
    i_state.pointer_velocity[0] =
        (xpos / win_w - i_state.prev_pointer[0]) / delta_time;
    i_state.pointer_velocity[1] =
        (1 - ypos / win_h - i_state.prev_pointer[1]) / delta_time;
  } else {
    i_state.pointer_velocity[0] = 0;
    i_state.pointer_velocity[1] = 0;
  }

  i_state.pointer[0] = xpos / win_w;
  i_state.pointer[1] = 1 - (ypos / win_h);
}

#define INPUT_MAPPING_PATH CONFIG_PATH("input.map")

void i_init() {
  // how can i zero-init this everywhere else?
  ActionType action_mapping[NUM_MODIFIER_PERMUTATIONS][INPUT_COUNT] = {
      [0] =
          {
              [KEY_W] = ACT_UP,
              [KEY_S] = ACT_DOWN,
              [KEY_D] = ACT_RIGHT,
              [KEY_A] = ACT_LEFT,
              [KEY_LEFT] = ACT_CAMERA_CW,
              [KEY_RIGHT] = ACT_CAMERA_CCW,
              [KEY_UP] = ACT_CAMERA_RAISE,
              [KEY_DOWN] = ACT_CAMERA_LOWER,
              [MOUSE_LEFT] = ACT_HUD_INTERACT,
          },
      [MOD_CONTROL] =
          {
              [KEY_C] = ACT_TOGGLE_DEBUG_CONSOLE,
              [KEY_D] = ACT_TOGGLE_DEBUG_DRAW,
          },
      [MOD_ALT] = {},
      [MOD_SUPER] = {},
  };

  // copy in the init mapping. TODO: maybe load this from a file at some point.
  memcpy(&(i_state.action_mapping), &action_mapping, sizeof(action_mapping));

  glfwSetKeyCallback(window, input_key_callback);
  glfwSetMouseButtonCallback(window, input_mouse_button_callback);
  glfwSetCursorPosCallback(window, input_cursor_position_callback);
}

void i_update() {
  memset(i_state.act_just_pressed, 0, sizeof(i_state.act_just_pressed));
  memset(i_state.act_just_released, 0, sizeof(i_state.act_just_released));
}

void i_clean() {}

// NOTE: add the mods together.
// for example, if i want CTRL+SHIFT+x as a command, i do
// map_input(KEY_X, MOD_CONTROL + MOD_SHIFT, ACT_BLAH);
void map_input(Input input, int mods, ActionType action) {
  i_state.action_mapping[mods][input] = action;
}
