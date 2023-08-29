#include "input.h"

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

static void handle_generic_input(int key, int action, int mods) {
#define MAP(key, input_action)                                                 \
  case key: {                                                                  \
    switch (action) {                                                          \
    case GLFW_PRESS:                                                           \
      if (!i_state.last_act_state[input_action]) {                             \
        i_state.act_just_pressed[input_action] = 1;                            \
      }                                                                        \
      i_state.act_held[input_action] = 1;                                      \
      break;                                                                   \
    case GLFW_RELEASE:                                                         \
      if (i_state.last_act_state[input_action]) {                              \
        i_state.act_just_released[input_action] = 1;                           \
      }                                                                        \
      i_state.act_held[input_action] = 0;                                      \
      break;                                                                   \
    default:                                                                   \
      break;                                                                   \
    }                                                                          \
    i_state.last_act_state[input_action] = i_state.act_held[input_action];     \
  }

#define IS_SHIFT (mods & GLFW_MOD_SHIFT)
#define IS_CTRL (mods & GLFW_MOD_CONTROL)

  switch (key) {
    MAP(GLFW_KEY_W, ACT_UP);
    MAP(GLFW_KEY_S, ACT_DOWN);
    MAP(GLFW_KEY_A, ACT_LEFT);
    MAP(GLFW_KEY_D, ACT_RIGHT);
    MAP(GLFW_KEY_SPACE, ACT_JUMP);
  }

  if (IS_CTRL) {
    switch (key) {
      MAP(GLFW_KEY_D, ACT_TOGGLE_DEBUG_DRAW);
      MAP(GLFW_KEY_C, ACT_TOGGLE_DEBUG_CONSOLE);
    }
  }

#undef IS_CTRL
#undef IS_SHIFT

#undef MAP
}

static void input_key_callback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  console_handle_input(key, scancode, action, mods);

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  handle_generic_input(key, action, mods);
}

static void input_mouse_button_callback(GLFWwindow *window, int button,
                                        int action, int mods) {
  handle_generic_input(button, action, mods);
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

void i_init() {
  glfwSetKeyCallback(window, input_key_callback);
  glfwSetMouseButtonCallback(window, input_mouse_button_callback);
  glfwSetCursorPosCallback(window, input_cursor_position_callback);
}

void i_update() {
  memset(i_state.act_just_pressed, 0, sizeof(i_state.act_just_pressed));
  memset(i_state.act_just_released, 0, sizeof(i_state.act_just_released));
}

void i_clean() {}
