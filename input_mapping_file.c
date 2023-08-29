#include "input_mapping_file.h"
#include <string.h>

static Input parse_string_to_input(const char *str) {
  if (strcmp(str, "KEY_SPACE") == 0)
    return KEY_SPACE;
  if (strcmp(str, "KEY_APOSTROPHE") == 0)
    return KEY_APOSTROPHE;
  if (strcmp(str, "KEY_COMMA") == 0)
    return KEY_COMMA;
  if (strcmp(str, "KEY_MINUS") == 0)
    return KEY_MINUS;
  if (strcmp(str, "KEY_PERIOD") == 0)
    return KEY_PERIOD;
  if (strcmp(str, "KEY_SLASH") == 0)
    return KEY_SLASH;
  if (strcmp(str, "KEY_0") == 0)
    return KEY_0;
  if (strcmp(str, "KEY_1") == 0)
    return KEY_1;
  if (strcmp(str, "KEY_2") == 0)
    return KEY_2;
  if (strcmp(str, "KEY_3") == 0)
    return KEY_3;
  if (strcmp(str, "KEY_4") == 0)
    return KEY_4;
  if (strcmp(str, "KEY_5") == 0)
    return KEY_5;
  if (strcmp(str, "KEY_6") == 0)
    return KEY_6;
  if (strcmp(str, "KEY_7") == 0)
    return KEY_7;
  if (strcmp(str, "KEY_8") == 0)
    return KEY_8;
  if (strcmp(str, "KEY_9") == 0)
    return KEY_9;
  if (strcmp(str, "KEY_SEMICOLON") == 0)
    return KEY_SEMICOLON;
  if (strcmp(str, "KEY_EQUAL") == 0)
    return KEY_EQUAL;
  if (strcmp(str, "KEY_A") == 0)
    return KEY_A;
  if (strcmp(str, "KEY_B") == 0)
    return KEY_B;
  if (strcmp(str, "KEY_C") == 0)
    return KEY_C;
  if (strcmp(str, "KEY_D") == 0)
    return KEY_D;
  if (strcmp(str, "KEY_E") == 0)
    return KEY_E;
  if (strcmp(str, "KEY_F") == 0)
    return KEY_F;
  if (strcmp(str, "KEY_G") == 0)
    return KEY_G;
  if (strcmp(str, "KEY_H") == 0)
    return KEY_H;
  if (strcmp(str, "KEY_I") == 0)
    return KEY_I;
  if (strcmp(str, "KEY_J") == 0)
    return KEY_J;
  if (strcmp(str, "KEY_K") == 0)
    return KEY_K;
  if (strcmp(str, "KEY_L") == 0)
    return KEY_L;
  if (strcmp(str, "KEY_M") == 0)
    return KEY_M;
  if (strcmp(str, "KEY_N") == 0)
    return KEY_N;
  if (strcmp(str, "KEY_O") == 0)
    return KEY_O;
  if (strcmp(str, "KEY_P") == 0)
    return KEY_P;
  if (strcmp(str, "KEY_Q") == 0)
    return KEY_Q;
  if (strcmp(str, "KEY_R") == 0)
    return KEY_R;
  if (strcmp(str, "KEY_S") == 0)
    return KEY_S;
  if (strcmp(str, "KEY_T") == 0)
    return KEY_T;
  if (strcmp(str, "KEY_U") == 0)
    return KEY_U;
  if (strcmp(str, "KEY_V") == 0)
    return KEY_V;
  if (strcmp(str, "KEY_W") == 0)
    return KEY_W;
  if (strcmp(str, "KEY_X") == 0)
    return KEY_X;
  if (strcmp(str, "KEY_Y") == 0)
    return KEY_Y;
  if (strcmp(str, "KEY_Z") == 0)
    return KEY_Z;
  if (strcmp(str, "KEY_LEFT_BRACKET") == 0)
    return KEY_LEFT_BRACKET;
  if (strcmp(str, "KEY_BACKSLASH") == 0)
    return KEY_BACKSLASH;
  if (strcmp(str, "KEY_RIGHT_BRACKET") == 0)
    return KEY_RIGHT_BRACKET;
  if (strcmp(str, "KEY_GRAVE_ACCENT") == 0)
    return KEY_GRAVE_ACCENT;
  if (strcmp(str, "KEY_WORLD_1") == 0)
    return KEY_WORLD_1;
  if (strcmp(str, "KEY_WORLD_2") == 0)
    return KEY_WORLD_2;
  if (strcmp(str, "KEY_ESCAPE") == 0)
    return KEY_ESCAPE;
  if (strcmp(str, "KEY_ENTER") == 0)
    return KEY_ENTER;
  if (strcmp(str, "KEY_TAB") == 0)
    return KEY_TAB;
  if (strcmp(str, "KEY_BACKSPACE") == 0)
    return KEY_BACKSPACE;
  if (strcmp(str, "KEY_INSERT") == 0)
    return KEY_INSERT;
  if (strcmp(str, "KEY_DELETE") == 0)
    return KEY_DELETE;
  if (strcmp(str, "KEY_RIGHT") == 0)
    return KEY_RIGHT;
  if (strcmp(str, "KEY_LEFT") == 0)
    return KEY_LEFT;
  if (strcmp(str, "KEY_DOWN") == 0)
    return KEY_DOWN;
  if (strcmp(str, "KEY_UP") == 0)
    return KEY_UP;
  if (strcmp(str, "KEY_PAGE_UP") == 0)
    return KEY_PAGE_UP;
  if (strcmp(str, "KEY_PAGE_DOWN") == 0)
    return KEY_PAGE_DOWN;
  if (strcmp(str, "KEY_HOME") == 0)
    return KEY_HOME;
  if (strcmp(str, "KEY_END") == 0)
    return KEY_END;
  if (strcmp(str, "KEY_CAPS_LOCK") == 0)
    return KEY_CAPS_LOCK;
  if (strcmp(str, "KEY_SCROLL_LOCK") == 0)
    return KEY_SCROLL_LOCK;
  if (strcmp(str, "KEY_NUM_LOCK") == 0)
    return KEY_NUM_LOCK;
  if (strcmp(str, "KEY_PRINT_SCREEN") == 0)
    return KEY_PRINT_SCREEN;
  if (strcmp(str, "KEY_PAUSE") == 0)
    return KEY_PAUSE;
  if (strcmp(str, "KEY_F1") == 0)
    return KEY_F1;
  if (strcmp(str, "KEY_F2") == 0)
    return KEY_F2;
  if (strcmp(str, "KEY_F3") == 0)
    return KEY_F3;
  if (strcmp(str, "KEY_F4") == 0)
    return KEY_F4;
  if (strcmp(str, "KEY_F5") == 0)
    return KEY_F5;
  if (strcmp(str, "KEY_F6") == 0)
    return KEY_F6;
  if (strcmp(str, "KEY_F7") == 0)
    return KEY_F7;
  if (strcmp(str, "KEY_F8") == 0)
    return KEY_F8;
  if (strcmp(str, "KEY_F9") == 0)
    return KEY_F9;
  if (strcmp(str, "KEY_F10") == 0)
    return KEY_F10;
  if (strcmp(str, "KEY_F11") == 0)
    return KEY_F11;
  if (strcmp(str, "KEY_F12") == 0)
    return KEY_F12;
  if (strcmp(str, "KEY_F13") == 0)
    return KEY_F13;
  if (strcmp(str, "KEY_F14") == 0)
    return KEY_F14;
  if (strcmp(str, "KEY_F15") == 0)
    return KEY_F15;
  if (strcmp(str, "KEY_F16") == 0)
    return KEY_F16;
  if (strcmp(str, "KEY_F17") == 0)
    return KEY_F17;
  if (strcmp(str, "KEY_F18") == 0)
    return KEY_F18;
  if (strcmp(str, "KEY_F19") == 0)
    return KEY_F19;
  if (strcmp(str, "KEY_F20") == 0)
    return KEY_F20;
  if (strcmp(str, "KEY_F21") == 0)
    return KEY_F21;
  if (strcmp(str, "KEY_F22") == 0)
    return KEY_F22;
  if (strcmp(str, "KEY_F23") == 0)
    return KEY_F23;
  if (strcmp(str, "KEY_F24") == 0)
    return KEY_F24;
  if (strcmp(str, "KEY_F25") == 0)
    return KEY_F25;
  if (strcmp(str, "KEY_KP_0") == 0)
    return KEY_KP_0;
  if (strcmp(str, "KEY_KP_1") == 0)
    return KEY_KP_1;
  if (strcmp(str, "KEY_KP_2") == 0)
    return KEY_KP_2;
  if (strcmp(str, "KEY_KP_3") == 0)
    return KEY_KP_3;
  if (strcmp(str, "KEY_KP_4") == 0)
    return KEY_KP_4;
  if (strcmp(str, "KEY_KP_5") == 0)
    return KEY_KP_5;
  if (strcmp(str, "KEY_KP_6") == 0)
    return KEY_KP_6;
  if (strcmp(str, "KEY_KP_7") == 0)
    return KEY_KP_7;
  if (strcmp(str, "KEY_KP_8") == 0)
    return KEY_KP_8;
  if (strcmp(str, "KEY_KP_9") == 0)
    return KEY_KP_9;
  if (strcmp(str, "KEY_KP_DECIMAL") == 0)
    return KEY_KP_DECIMAL;
  if (strcmp(str, "KEY_KP_DIVIDE") == 0)
    return KEY_KP_DIVIDE;
  if (strcmp(str, "KEY_KP_MULTIPLY") == 0)
    return KEY_KP_MULTIPLY;
  if (strcmp(str, "KEY_KP_SUBTRACT") == 0)
    return KEY_KP_SUBTRACT;
  if (strcmp(str, "KEY_KP_ADD") == 0)
    return KEY_KP_ADD;
  if (strcmp(str, "KEY_KP_ENTER") == 0)
    return KEY_KP_ENTER;
  if (strcmp(str, "KEY_KP_EQUAL") == 0)
    return KEY_KP_EQUAL;
  if (strcmp(str, "KEY_LEFT_SHIFT") == 0)
    return KEY_LEFT_SHIFT;
  if (strcmp(str, "KEY_LEFT_CONTROL") == 0)
    return KEY_LEFT_CONTROL;
  if (strcmp(str, "KEY_LEFT_ALT") == 0)
    return KEY_LEFT_ALT;
  if (strcmp(str, "KEY_LEFT_SUPER") == 0)
    return KEY_LEFT_SUPER;
  if (strcmp(str, "KEY_RIGHT_SHIFT") == 0)
    return KEY_RIGHT_SHIFT;
  if (strcmp(str, "KEY_RIGHT_CONTROL") == 0)
    return KEY_RIGHT_CONTROL;
  if (strcmp(str, "KEY_RIGHT_ALT") == 0)
    return KEY_RIGHT_ALT;
  if (strcmp(str, "KEY_RIGHT_SUPER") == 0)
    return KEY_RIGHT_SUPER;
  if (strcmp(str, "KEY_MENU") == 0)
    return KEY_MENU;
  if (strcmp(str, "MOUSE_LEFT") == 0)
    return MOUSE_LEFT;
  if (strcmp(str, "MOUSE_MIDDLE") == 0)
    return MOUSE_MIDDLE;
  if (strcmp(str, "MOUSE_RIGHT") == 0)
    return MOUSE_RIGHT;

  // If you reached here, the string didn't match any key.
  return INPUT_COUNT;
}
