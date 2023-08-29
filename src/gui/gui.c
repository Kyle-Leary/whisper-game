#include "gui.h"
#include "cglm/affine-pre.h"
#include "cglm/affine.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "gui/gui_layouts.h"
#include "gui/gui_renders.h"
#include "gui/widgets.h"
#include "helper_math.h"
#include "input/input.h"
#include "meshing/font.h"
#include "path.h"
#include "printers.h"
#include "render/gl_util.h"
#include "render/graphics_render.h"
#include "render/render_configuration.h"
#include "shaders/shader.h"
#include "shaders/shader_binding.h"
#include "shaders/shader_instances.h"
#include "transform.h"
#include "whisper/array.h"
#include "whisper/colmap.h"
#include "whisper/hashmap.h"
#include "whisper/macros.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "macros.h"

#include "ogl_includes.h"
#include "whisper/stack.h"

GUIState gui_state = {0};

// execute "block" with the widget context ptr "w".
#define WIDGET_MAP_FORALL(widget_t, widget_colmap, ...)                        \
  {                                                                            \
    WColMap *cm = &(gui_state.widget_colmap);                                  \
    for (int i = 0; i < cm->num_elms; i++) {                                   \
      widget_t *w = w_array_get(cm, i);                                        \
      if (w && w->is_in_use) {                                                 \
        __VA_ARGS__                                                            \
      }                                                                        \
    }                                                                          \
  }

// wrapper macro with all the widget types defined.
#define ALL_WIDGETS_DO(block)                                                  \
  {                                                                            \
    WIDGET_MAP_FORALL(GUIWidget, widgets, block);                              \
    WIDGET_MAP_FORALL(GUILabel, labels, block);                                \
    WIDGET_MAP_FORALL(GUIButton, buttons, block);                              \
    WIDGET_MAP_FORALL(GUIDraggable, draggables, block);                        \
  }

void gui_init() {
  gui_render_init();
  gui_widget_types_init();
  layout_init();

  // specify max window depth here. we're making a stack of pointers, nothing
  // will actually be stored in the stack itself. it's just a context
  // organizational structure.
  w_stack_create(&(gui_state.window_stack), sizeof(GUIWidget *),
                 MAX_WINDOW_DEPTH);

  { // make the root widget element.
    GUIWidget **root_ptr_ptr =
        (GUIWidget **)w_stack_push(&(gui_state.window_stack));

    GUIWidget stack_root = {0};
    stack_root.type = WT_WIDGET;
    stack_root.is_in_use = true;
    memcpy(&(stack_root.aabb), &(AABB){0.5, 0.5, 0.5, 0.5}, sizeof(AABB));

    GUIWidget *root_widget_slot =
        w_cm_insert(&(gui_state.widgets), "root widget", &stack_root);
    memcpy(root_ptr_ptr, &root_widget_slot, sizeof(void *));
  }
}

void gui_clean() {
  w_clean_array(&(gui_state.widgets));
  w_clean_array(&(gui_state.buttons));
  w_clean_array(&(gui_state.labels));
  w_clean_array(&(gui_state.draggables));

  w_stack_clean(&(gui_state.window_stack));
}

// add the last_added widget to the current parent in scope.
void gui_internal_add_child(GUIWidget *last_added) {
  // grab a pointer to the actual parent on the stack, and deref the pointer so
  // that it's pointing to the actual element itself.
  GUIWidget *ptr =
      *(GUIWidget **)((uint8_t *)gui_state.window_stack.stack_pointer -
                      gui_state.window_stack.elm_sz);

  // doubly link the tree entries.
  ptr->children[ptr->num_children] = last_added;
  ptr->num_children++;
  last_added->parent = ptr;

  // specifically accept the LOCAL transform.
  layout_accept_new(&(last_added->aabb));
}

void gui_internal_push(GUIWidget *last_added) {
  NULL_CHECK(last_added);

  GUIWidget **w = w_stack_push(&(gui_state.window_stack));
  memcpy(w, &last_added, sizeof(void *));
  gui_state.is_pushing = false;

  layout_internal_push();
}

inline static GUIWidget *gui_get_root() {
  return *(GUIWidget **)gui_state.window_stack.base_pointer;
}

// call gui functions after the gui update. the gui_update function basically
// serves as a state reset for the immediate mode context.
void gui_update() {
  ALL_WIDGETS_DO({ w->is_in_use = false; })

  gui_state.last_added = NULL;

  GUIWidget *base_case = gui_get_root();
  RUNTIME_ASSERT(base_case->parent == NULL);

  // offset the stack pointer one above the root, so that everything uses the
  // root as a direct parent.
  gui_state.window_stack.stack_pointer =
      (void *)((uint8_t *)gui_state.window_stack.base_pointer +
               gui_state.window_stack.elm_sz);

  // then, reset the layout stack.
  layout_reset();
}

// TODO: set the layout on a push. store layout data on the stack.
void gui_push(Layout *layout) {
  gui_state.is_pushing = true;
  layout_push(layout);
}

void gui_pop() {
  gui_state.last_added = w_stack_pop(&(gui_state.window_stack));
  NULL_CHECK(gui_state.last_added);
  layout_pop();
}

// after drawing and handling all the widget states, we need to clean the child
// arrays in each widget so that the next frame can be processed and setup
// seperately all over again. we'll recurse downward for the drawing/transform
// composition pass, and then once we reach the leaves, we'll recurse upward
// until we don't find another parent.
static void gui_bubble_up(GUIWidget *ptr, GUIInputState *gui_input) {
  GUIInputState local_gui_input;
  memcpy(&local_gui_input, gui_input, sizeof(GUIInputState));

  bool is_clicking = false;

  local_gui_input.mouse_inside =
      is_point_inside(ptr->global_aabb, i_state.pointer);

  WidgetType t = ptr->type;
  if (IS_VALID_WIDGET_TYPE(t)) {
    char buf[256];
    sprintf(buf, "invalid widget type in the update switch, found %d",
            ptr->type);
    ERROR_FROM_BUF(buf);
  } else {
    widget_handlers[t].update(ptr, &local_gui_input);
  }

  ptr->num_children = 0;
  if (ptr->parent && ptr->parent->num_children != 0) {
    gui_bubble_up(ptr->parent, gui_input);
  }
}

static float get_center_axis(vec2 to_center, AABB *by, int index) {
  float a = (by->center[index] - by->extents[index]);
  float b = (by->center[index] + by->extents[index]);
  float t = to_center[index];

  float lerp = ((1 - t) * a) + (t * b);

  return lerp;
}

static float get_extent_axis(vec2 to_extents, vec2 by_extents, int index) {
  float new_extent = (by_extents[index] * 2) * (to_extents[index]);
  return new_extent;
}

static void get_new_center(AABB *by, vec2 to, vec2 dest) {
  dest[0] = to[0] * by->extents[0];
  dest[1] = to[1] * by->extents[1];
}

static void apply_transform(AABB *to, AABB *by, AABB *dest) {
  dest->center[0] = get_center_axis(to->center, by, 0);
  dest->center[1] = get_center_axis(to->center, by, 1);

  dest->extents[0] = get_extent_axis(to->extents, by->extents, 0);
  dest->extents[1] = get_extent_axis(to->extents, by->extents, 1);

#undef CENTER_AXIS
}

static void gui_draw_recursive(GUIWidget *ptr, AABB transform) {

  // apply then recursively propagate the temp through the tree.
  apply_transform(&(ptr->aabb), &transform, &(ptr->global_aabb));

  WidgetType t = ptr->type;
  if (IS_VALID_WIDGET_TYPE(t)) {
    char buf[256];
    sprintf(buf, "invalid widget type in the draw switch, found %d", ptr->type);
    ERROR_FROM_BUF(buf);
  } else {
    widget_handlers[t].draw(ptr);
  }

  int num_child_renders = ptr->num_children;

  // gui_draw_recursive will mutate the ptr->num_children, so we need to be
  // careful in saving all that data so that all children are drawn and
  // iterated through.
  for (int i = 0; i < num_child_renders; i++) {
    GUIWidget *child_ptr = ptr->children[i];
    gui_draw_recursive(child_ptr, ptr->global_aabb);
  }

  if (ptr->num_children == 0) {
    // leaf node of the tree.
    GUIInputState gui_input = {
        .mouse_button = {i_state.act_held[ACT_HUD_INTERACT], false, false},
        .mouse_button_just_clicked = {
            i_state.act_just_pressed[ACT_HUD_INTERACT], false, false}};
    gui_bubble_up(ptr, &gui_input);
  }
}

void gui_draw() {
  mat4 model;
  glm_mat4_identity(model);

  shader_bind(render_state.shader);

  GUIWidget *base_case = gui_get_root();
  RUNTIME_ASSERT(base_case->parent == NULL);

  gui_draw_recursive(base_case, base_case->aabb);
}
