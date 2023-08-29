#pragma once

#include "helper_math.h"

typedef void (*GUIFunction)();

typedef struct GUIFunctionListSingleInput {
  GUIFunction function;
  char *name;
} GUIFunctionListSingleInput;

typedef struct GUIFunctionListInput {
  GUIFunctionListSingleInput inputs[10];
  int num_inputs;
} GUIFunctionListInput;

void gui_function_list(GUIFunctionListInput *input);
