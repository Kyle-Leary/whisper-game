#include "gui_layouts.h"
#include "gui/gui.h"
#include "helper_math.h"

#include "macros.h"
#include "whisper/macros.h"
#include "whisper/stack.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static LayoutHorizontal default_layout = {LAYOUT_HORIZONTAL, 0, 0};

// this is a buffer that represents ANY layout. cast this to Layout* and check
// the type when you're ready to use it.
uint8_t layout_buf[2][LAYOUT_BUF_SZ] = {0};
// alias the buffer for easy access. this is safe since the buffer pointer
// location won't change.
Layout *curr_layout = (Layout *)&(layout_buf[1]);

typedef struct LayoutState {
  // all of the transforms, in order, currently being laid out by the layout
  // subsystem.
  GUIWidget *layout_subjects[MAX_CHILDREN];
  int num_layout_subjects;
} LayoutState;

// this is a stack of layout states that works in parallel to the normal gui
// window stack.
static WStack layout_stack;

typedef void (*layout_fn)(GUIWidget *children[MAX_CHILDREN], int num_children,
                          Layout *layout);

void vertical_handler(GUIWidget *children[MAX_CHILDREN], int num_children,
                      Layout *layout) {

  LayoutVertical *v = (LayoutVertical *)layout;

  float y_offset = 0.5 / num_children;

  for (int i = 0; i < num_children; i++) {
    float y = (float)i / num_children;
    y += y_offset;

    AABB tf = {0.5, 1 - y, (0.5 - v->padding),
               (0.5 - v->padding) / num_children};
    memcpy(&children[i]->aabb, &tf, sizeof(AABB));
  }
}

void horizontal_handler(GUIWidget *children[MAX_CHILDREN], int num_children,
                        Layout *layout) {
  LayoutHorizontal *h = (LayoutHorizontal *)layout;

  float x_offset = 0.5 / num_children;

  for (int i = 0; i < num_children; i++) {
    switch (children[i]->type) {
    case WT_DRAGGABLE: {
      if (((GUIDraggable *)children[i])->has_moved) {
        continue;
      }
    } break;

    default: {
    } break;
    }

    float x = (float)i / num_children;
    x += x_offset;

    AABB tf = {x, 0.5, (0.5 - h->padding) / num_children, (0.5 - h->padding)};
    memcpy(&children[i]->aabb, &tf, sizeof(AABB));
  }
}

int layout_sizes[LAYOUT_COUNT] = {
    [LAYOUT_VERTICAL] = sizeof(LayoutVertical),
    [LAYOUT_HORIZONTAL] = sizeof(LayoutHorizontal),
};

layout_fn layout_handlers[LAYOUT_COUNT] = {
    [LAYOUT_VERTICAL] = vertical_handler,
    [LAYOUT_HORIZONTAL] = horizontal_handler,
};

// push into the first buffer, then load into the actual second buffer after the
// next gui widget has been added.
void layout_push(Layout *layout) {
  if (layout) {
    memcpy(&(layout_buf[0]), layout, layout_sizes[layout->type]);
  } else {
    // use a default layout when NULL.
    memcpy(&(layout_buf[0]), &default_layout, sizeof(LayoutHorizontal));
  }
}

inline static LayoutState *get_curr_layout_state() {
  LayoutState *curr = (LayoutState *)((uint8_t *)(layout_stack.stack_pointer) -
                                      layout_stack.elm_sz);
  return curr;
}

void layout_internal_push() {
  // make a new layout stack frame.
  LayoutState *new = w_stack_push(&layout_stack);
  new->num_layout_subjects = 0;
  // push the layout into the internal layout.
  memcpy(&(layout_buf[1]), &(layout_buf[0]),
         layout_sizes[((Layout *)(&layout_buf[0]))->type]);
}

// how can we restore the previous context? should this entire subsystem store
// its state in a parallel stack to the normal gui stack?
void layout_pop() { w_stack_pop(&layout_stack); }

void layout_reset() {
  w_stack_pop_all(&layout_stack);
  LayoutState *curr = get_curr_layout_state();
  curr->num_layout_subjects = 0;
}

void layout_accept_new(GUIWidget *new_widget) {
  LayoutState *curr = get_curr_layout_state();

  RUNTIME_ASSERT(new_widget->type < WT_COUNT);

  // store the REFERENCE to the transform for us to modify directly.
  curr->layout_subjects[curr->num_layout_subjects] = new_widget;
  curr->num_layout_subjects++;
  RUNTIME_ASSERT(curr->num_layout_subjects < MAX_CHILDREN);
  // now, reorganize all the transforms based on the new information.
  layout_handlers[curr_layout->type](curr->layout_subjects,
                                     curr->num_layout_subjects, curr_layout);
}

void layout_init() {
  w_stack_create(&layout_stack, sizeof(LayoutState), MAX_WINDOW_DEPTH);
  memcpy(curr_layout, &default_layout, sizeof(default_layout));
}

void layout_clean() { w_stack_clean(&layout_stack); }
