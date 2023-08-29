#include "gui_prim.h"
#include "gui/gui.h"
#include "gui/widgets.h"
#include "helper_math.h"
#include "printers.h"

// generate buttons for a list of void functions.
void gui_function_list(GUIFunctionListInput *input) {
  int max = input->num_inputs;
  for (int i = 0; i < max; i++) {
    GUIFunctionListSingleInput single_input = input->inputs[i];
    char *name = single_input.name;

    if (gui_button(name, name)) {
      single_input.function();
    }
  }
}
