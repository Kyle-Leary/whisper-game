#pragma once

#include "gui/gui_layouts.h"
#include "gui_renders.h"
#include "helper_math.h"
#include "whisper/colmap.h"
#include "whisper/stack.h"
#include <sys/types.h>

#define MAX_WINDOW_DEPTH 16

// this will bubble up throughout the gui widget tree, and be passed to parents
// to have a "child consumes parent input" type gui.
typedef struct GUIInputState {
  bool mouse_button_just_clicked[3];
  bool mouse_button[3];
  bool mouse_inside;
} GUIInputState;

typedef struct GUIWidget GUIWidget;

// takes in the transform and the widget being operated on.
typedef void (*draw_fn)(GUIWidget *);
// takes in the bubbled up local input pointer.
typedef void (*update_fn)(GUIWidget *, GUIInputState *);

typedef enum WidgetType {
  WT_INVALID = 0,
  WT_WIDGET,
  WT_LABEL,
  WT_DRAGGABLE,
  WT_BUTTON,

  WT_COUNT,
} WidgetType;

#define IS_VALID_WIDGET_TYPE(t) ((t >= WT_COUNT) || (t <= WT_INVALID))

typedef struct WidgetHandler {
  // both fn pointers here are called in the recursive up and down loops over
  // the widget tree structure.

  // we draw down from the root
  draw_fn draw;
  // and update up from the leaves.
  update_fn update;
} WidgetHandler;

extern WidgetHandler widget_handlers[WT_COUNT];

#define MAX_CHILDREN 10

// highest z_index is on the top.
//
// allow the layout to control the aabb, the recursion to control the
// global_aabb and the widget itself to control its relative position through
// the offset.
#define WIDGET_COMMON                                                          \
  WidgetType type;                                                             \
  GUIRender render;                                                            \
  bool is_in_use;                                                              \
  int z_index;                                                                 \
  GUIWidget *parent;                                                           \
  GUIWidget *children[MAX_CHILDREN];                                           \
  uint num_children;                                                           \
  AABB aabb;                                                                   \
  AABB global_aabb;                                                            \
  vec2 offset;

typedef struct GUIWidget {
  WIDGET_COMMON
} GUIWidget;

typedef struct GUILabel {
  WIDGET_COMMON
  char buffer[256];
  int buf_len;
} GUILabel;

// just a draggable quad. this always has 6 indices.
typedef struct GUIDraggable {
  WIDGET_COMMON
} GUIDraggable;

typedef struct GUIButton {
  WIDGET_COMMON
  char buffer[256];
  int buf_len;
  bool is_clicked;
} GUIButton;

#define GUI_MOUSE_LEFT 0
#define GUI_MOUSE_MIDDLE 1
#define GUI_MOUSE_RIGHT 2

typedef struct GUIState {
  WColMap widgets;
  WColMap labels;
  WColMap buttons;
  WColMap draggables;

  WStack window_stack;
  GUIWidget *last_added;
  bool is_pushing; // should we push on the next gui widget add?
} GUIState;

extern GUIState gui_state;

// pass NULL to use a default layout.
void gui_push(Layout *layout);
void gui_pop();

void gui_internal_push(GUIWidget *last_added);
void gui_internal_add_child(GUIWidget *last_added);

// lifecycle
void gui_init();
void gui_update();
void gui_draw();
void gui_clean();
