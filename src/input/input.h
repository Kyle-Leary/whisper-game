#ifndef INPUT_H
#define INPUT_H

// i_ is the namespace for the input api.

#include "cglm/cglm.h"
#include "cglm/types.h"
#include "defines.h"
#include <stdbool.h>

// mostly ripped from GLFW's input mappings, so converting from that to this is
// cheap.
typedef enum Input {
  KEY_SPACE = 32,
  KEY_APOSTROPHE = 39,
  KEY_COMMA = 44,
  KEY_MINUS = 45,
  KEY_PERIOD = 46,
  KEY_SLASH = 47,

  KEY_0 = 48,
  KEY_1 = 49,
  KEY_2 = 50,
  KEY_3 = 51,
  KEY_4 = 52,
  KEY_5 = 53,
  KEY_6 = 54,
  KEY_7 = 55,
  KEY_8 = 56,
  KEY_9 = 57,

  KEY_SEMICOLON = 59 /* ; */,
  KEY_EQUAL = 61 /* = */,

  // GLFW and this uses ASCII codes as the #define variants for keycodes.
  KEY_A = 65,
  KEY_B = 66,
  KEY_C = 67,
  KEY_D = 68,
  KEY_E = 69,
  KEY_F = 70,
  KEY_G = 71,
  KEY_H = 72,
  KEY_I = 73,
  KEY_J = 74,
  KEY_K = 75,
  KEY_L = 76,
  KEY_M = 77,
  KEY_N = 78,
  KEY_O = 79,
  KEY_P = 80,
  KEY_Q = 81,
  KEY_R = 82,
  KEY_S = 83,
  KEY_T = 84,
  KEY_U = 85,
  KEY_V = 86,
  KEY_W = 87,
  KEY_X = 88,
  KEY_Y = 89,
  KEY_Z = 90,

  KEY_LEFT_BRACKET = 91 /* [ */,
  KEY_BACKSLASH = 92 /* \ */,
  KEY_RIGHT_BRACKET = 93 /* ] */,
  KEY_GRAVE_ACCENT = 96 /* ` */,

  KEY_WORLD_1 = 161 /* non-US #1 */,
  KEY_WORLD_2 = 162 /* non-US #2 */,
  KEY_ESCAPE = 256,
  KEY_ENTER = 257,
  KEY_TAB = 258,
  KEY_BACKSPACE = 259,
  KEY_INSERT = 260,
  KEY_DELETE = 261,
  KEY_RIGHT = 262,
  KEY_LEFT = 263,
  KEY_DOWN = 264,
  KEY_UP = 265,
  KEY_PAGE_UP = 266,
  KEY_PAGE_DOWN = 267,
  KEY_HOME = 268,
  KEY_END = 269,
  KEY_CAPS_LOCK = 280,
  KEY_SCROLL_LOCK = 281,
  KEY_NUM_LOCK = 282,
  KEY_PRINT_SCREEN = 283,
  KEY_PAUSE = 284,
  KEY_F1 = 290,
  KEY_F2 = 291,
  KEY_F3 = 292,
  KEY_F4 = 293,
  KEY_F5 = 294,
  KEY_F6 = 295,
  KEY_F7 = 296,
  KEY_F8 = 297,
  KEY_F9 = 298,
  KEY_F10 = 299,
  KEY_F11 = 300,
  KEY_F12 = 301,
  KEY_F13 = 302,
  KEY_F14 = 303,
  KEY_F15 = 304,
  KEY_F16 = 305,
  KEY_F17 = 306,
  KEY_F18 = 307,
  KEY_F19 = 308,
  KEY_F20 = 309,
  KEY_F21 = 310,
  KEY_F22 = 311,
  KEY_F23 = 312,
  KEY_F24 = 313,
  KEY_F25 = 314,
  KEY_KP_0 = 320,
  KEY_KP_1 = 321,
  KEY_KP_2 = 322,
  KEY_KP_3 = 323,
  KEY_KP_4 = 324,
  KEY_KP_5 = 325,
  KEY_KP_6 = 326,
  KEY_KP_7 = 327,
  KEY_KP_8 = 328,
  KEY_KP_9 = 329,
  KEY_KP_DECIMAL = 330,
  KEY_KP_DIVIDE = 331,
  KEY_KP_MULTIPLY = 332,
  KEY_KP_SUBTRACT = 333,
  KEY_KP_ADD = 334,
  KEY_KP_ENTER = 335,
  KEY_KP_EQUAL = 336,
  KEY_LEFT_SHIFT = 340,
  KEY_LEFT_CONTROL = 341,
  KEY_LEFT_ALT = 342,
  KEY_LEFT_SUPER = 343,
  KEY_RIGHT_SHIFT = 344,
  KEY_RIGHT_CONTROL = 345,
  KEY_RIGHT_ALT = 346,
  KEY_RIGHT_SUPER = 347,
  KEY_MENU = 348,

  MOUSE_LEFT = 350,
  MOUSE_MIDDLE,
  MOUSE_RIGHT,

  INPUT_COUNT,
} Input;

#define MOD_SHIFT 0x0001
#define MOD_CONTROL 0x0002
#define MOD_ALT 0x0004
#define MOD_SUPER 0x0008

#define NUM_MODIFIER_PERMUTATIONS                                              \
  (MOD_SHIFT + MOD_CONTROL + MOD_ALT + MOD_SUPER)

// there are ActionTypes, which are on-and-off states stored in a simple array.
// then, for stuff like the joystick, there are special inputs in the InputState
// structure for those.
typedef enum ActionType {
  ACT_ZERO, // lol, because of the way this is implemented, the 0th act is
            // dangerous to use. so just assign it to a fake action and dump it.
  ACT_UP,
  ACT_DOWN,
  ACT_LEFT,
  ACT_RIGHT,

  ACT_JUMP,

  ACT_CAMERA_CW,  // swing cw around the player
  ACT_CAMERA_CCW, // swing ccw around the player

  ACT_CAMERA_RAISE,
  ACT_CAMERA_LOWER,

  ACT_WORLD_INTERACT,

  ACT_IS_SWINGING, // eg, the mouse is held down to init a swing on computer.

  ACT_STRAFE_LEFT,
  ACT_STRAFE_RIGHT,

  ACT_HUD_INTERACT, // hud interaction action, like clicking on a button.

  ACT_TOGGLE_DEBUG_DRAW,
  ACT_TOGGLE_DEBUG_CONSOLE,

  ACT_INCREASE_TIMESCALE,
  ACT_DECREASE_TIMESCALE,

  ACT_SCREENSHOT,

  ACT_COUNT, // NO, COUNT IS NOT AN ACTION!!!
} ActionType;

// everything shares a common input state, but has different update files that
// setup and flip the input state bits, depending on the input handlers.
typedef struct InputState {
  /// define etc inputs, like sticks and weird datapoints
  float stick_x;
  float stick_y;

  // either the mouse or an arbitrary "pointer" object
  // for the screen.
  vec2 pointer;
  vec2 pointer_velocity; // helper
  vec2 prev_pointer;     // we're operating on this in batch, all the pointer
                     // characteristics in a row. thus, it makes sense to store
                     // them all contiguously.

  /// then, define button-like binary inputs
  // for example, if (act_held[ACT_UP]) { // ACT_UP is being held }
  u8 act_held[ACT_COUNT];
  u8 act_just_pressed[ACT_COUNT];
  u8 act_just_released[ACT_COUNT];

  // the 0th array is no modifier, the MOD_CONTROL'th array is the control
  // modifier mapping, etc.
  // for fast access, we keep a seperate list for all the modifier permutations.
  ActionType action_mapping[NUM_MODIFIER_PERMUTATIONS][INPUT_COUNT];
} InputState;

extern InputState i_state;

void i_init();
void i_update();
void i_clean();

void map_input(Input input, int mods, ActionType action);

#endif
