#include "gui/gui.h"
#include "cglm/types.h"
#include "gui/gui_layouts.h"
#include "gui/gui_prim.h"
#include "gui/widgets.h"
#include "math/mat.h"
#include "objects/camera.h"
#include "path.h"
#include "render/light.h"
#include "render/model.h"
#include "render/render.h"

void init() {}

void function_one() { printf("function_one\n"); }
void function_two() { printf("function_two\n"); }

void update() {
  gui_vert_push(0.1, 0.1);
  gui_widget("v");

  gui_widget("h");

  gui_horiz_push(0.01, 0.09);
  gui_widget("wa");

  GUIFunctionListInput input = {
      .num_inputs = 2,
      .inputs =
          {
              {function_one, "a"},
              {function_two, "b"},
          },
  };
  gui_function_list(&input);

  gui_pop();

  {
    gui_push((Layout *)&(LayoutVertical){
        .type = LAYOUT_VERTICAL, .margin = 0.01, .padding = 0.09});
    gui_widget("subwindow");
    gui_label("top left", "top left");
    gui_label("top right", "top right");
    gui_label("bottom left", "bottom left");
    gui_label("bottom right", "bottom right");
    gui_pop();
  }
  gui_pop();
}
