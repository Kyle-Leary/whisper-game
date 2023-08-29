#include "gui.h"
#include "cglm/affine-pre.h"
#include "cglm/affine.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
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

typedef struct GUIRender {
  uint vao;
  uint n_idx;
} GUIRender;

// tree of gui widgets. the memory layout is just a hashmap, but they're linked
// through child pointer lists.
typedef struct GUIWidget GUIWidget;

typedef enum WidgetType {
  WT_INVALID = 0,
  WT_WIDGET,
  WT_LABEL,
  WT_DRAGGABLE,
  WT_BUTTON,

  WT_COUNT,
} WidgetType;

#define MAX_CHILDREN 10

// highest z_index is on the top.
#define WIDGET_COMMON                                                          \
  WidgetType type;                                                             \
  GUIRender render;                                                            \
  bool is_in_use;                                                              \
  int z_index;                                                                 \
  GUIWidget *parent;                                                           \
  GUIWidget *children[MAX_CHILDREN];                                           \
  uint num_children;                                                           \
  AABB aabb;

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
} GUIButton;

typedef struct GUIState {
  WColMap widgets;
  WColMap labels;
  WColMap buttons;
  WColMap draggables;

  WStack window_stack;
  GUIWidget *last_added;
  bool is_pushing; // should we push on the next gui widget add?
} GUIState;

static GUIState gui_state = {0};

static int default_z_index = 0;

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

static Font *gui_font;
static Shader *gui_program;
static mat4 g_projection;

void gui_init() {
  w_create_cm(&(gui_state.widgets), sizeof(GUIWidget), 256);
  w_create_cm(&(gui_state.labels), sizeof(GUILabel), 256);
  w_create_cm(&(gui_state.buttons), sizeof(GUIButton), 256);
  w_create_cm(&(gui_state.draggables), sizeof(GUIDraggable), 256);

  // specify max window depth here. we're making a stack of pointers, nothing
  // will actually be stored in the stack itself. it's just a context
  // organizational structure.
  w_stack_create(&(gui_state.window_stack), sizeof(GUIWidget *), 16);

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

  gui_font =
      font_init(16, 16, textures[g_load_texture(TEXTURE_PATH("ui_font.png"))]);

  { // init mats/global shader ref from the shader subsystem.
    glm_ortho(0, 1, 0, 1, 0.01, 100, g_projection);

    gui_program = get_shader("gui");
    shader_bind(gui_program);
    shader_set_matrix4fv(gui_program, "u_projection", (float *)g_projection);
  }
}

static void gui_make_render(GUIRender *gui_render, float *positions, float *uvs,
                            unsigned int *indices, uint num_indices,
                            uint num_verts) { // init test render in opengl.
  gui_render->n_idx = num_indices;
  gui_render->vao = make_vao();
  make_ebo(indices, num_indices);

  make_vbo(positions, num_verts, sizeof(float) * 2);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
  glEnableVertexAttribArray(0);

  make_vbo(uvs, num_verts, sizeof(float) * 2);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

static void gui_quad_render(GUIRender *gui_render) {
  // Vertex positions (x, y)
  float positions[8] = {
      -0.5f, -0.5f, // Bottom-left corner
      0.5f,  -0.5f, // Bottom-right corner
      0.5f,  0.5f,  // Top-right corner
      -0.5f, 0.5f   // Top-left corner
  };

  // UV coordinates (u, v)
  float uvs[8] = {
      0.0f, 0.0f, // Bottom-left corner
      1.0f, 0.0f, // Bottom-right corner
      1.0f, 1.0f, // Top-right corner
      0.0f, 1.0f  // Top-left corner
  };

  // Indices for the quad
  uint indices[6] = {
      0, 1, 2, // First triangle (bottom-left -> bottom-right -> top-right)
      2, 3, 0  // Second triangle (top-right -> top-left -> bottom-left)
  };

  gui_make_render(gui_render, positions, uvs, indices, 6, 4);
}

static void gui_string_render(GUIRender *gui_render,
                              const char *str) { // init test render.
  uint len = strlen(str);

  int num_verts = len * 4;
  int num_indices = len * 6;

  float positions[num_verts * 2];
  float uvs[num_verts * 2];
  uint indices[num_indices];

  font_mesh_string_raw(gui_font, str, len, 1, 1, positions, uvs, indices);

  gui_make_render(gui_render, positions, uvs, indices, num_indices, num_verts);
}

// add the last_added widget to the current parent in scope.
static void gui_add_child(GUIWidget *last_added) {
  // grab a pointer to the actual parent on the stack, and deref the pointer so
  // that it's pointing to the actual element itself.
  GUIWidget *ptr =
      *(GUIWidget **)((uint8_t *)gui_state.window_stack.stack_pointer -
                      gui_state.window_stack.elm_sz);

  // doubly link the tree entries.
  ptr->children[ptr->num_children] = last_added;
  ptr->num_children++;
  last_added->parent = ptr;
}

static void gui_actual_push(GUIWidget *last_added) {
  NULL_CHECK(last_added);

  GUIWidget **w = w_stack_push(&(gui_state.window_stack));
  memcpy(w, &last_added, sizeof(void *));
  gui_state.is_pushing = false;
}

#define DEFAULT_Z_INDEX()                                                      \
  {                                                                            \
    ptr->z_index = default_z_index;                                            \
    default_z_index++;                                                         \
  }

#define END_WIDGET()                                                           \
  {                                                                            \
    ptr->is_in_use = true;                                                     \
    gui_state.last_added = (GUIWidget *)ptr;                                   \
    gui_add_child((GUIWidget *)ptr);                                           \
    if (gui_state.is_pushing) {                                                \
      gui_actual_push((GUIWidget *)ptr);                                       \
    }                                                                          \
  }

void gui_widget(const char *name, AABB *aabb) {
  GUIWidget *ptr = w_cm_return_slot(&(gui_state.widgets), name);

  if (ptr) {
    // first time init
    ptr->type = WT_WIDGET;
    DEFAULT_Z_INDEX();
    memcpy(&(ptr->aabb), aabb, sizeof(AABB));
  } else {
    // else, it's returned NULL and we need to grab the slot ourselves.
    ptr = w_cm_get(&(gui_state.widgets), name);
  }

  END_WIDGET();
}

void gui_label(const char *name, const char *text, AABB *aabb) {
#define LABEL_RERENDER()                                                       \
  {                                                                            \
    gui_string_render(&(ptr->render), text);                                   \
    int len = strlen(text);                                                    \
    memcpy(ptr->buffer, text, len);                                            \
    ptr->buf_len = len;                                                        \
  }

  GUILabel *ptr = w_cm_return_slot(&(gui_state.labels), name);

  if (ptr) {
    ptr->type = WT_LABEL;
    DEFAULT_Z_INDEX();
    LABEL_RERENDER();
    memcpy(&(ptr->aabb), aabb, sizeof(AABB));
  } else {
    ptr = w_cm_get(&(gui_state.labels), name);

    if (strncmp(text, ptr->buffer, ptr->buf_len) != 0) {
      // the text has changed.
      LABEL_RERENDER();
    }
  }

  END_WIDGET();

#undef LABEL_RERENDER
}

bool gui_button(const char *name, const char *text, AABB *aabb) {
  GUIButton *ptr = w_cm_return_slot(&(gui_state.buttons), name);

  if (ptr) {
    ptr->type = WT_BUTTON;

    // new insertion.
    gui_string_render(&(ptr->render), text);

    int len = strlen(text);
    memcpy(ptr->buffer, text, len);
    DEFAULT_Z_INDEX();
    ptr->buf_len = len;
    memcpy(&(ptr->aabb), aabb, sizeof(AABB));

  } else {

    ptr = w_cm_get(&(gui_state.buttons), name);
  }

  END_WIDGET();

  if (i_state.act_just_pressed[ACT_HUD_INTERACT]) {
    return (is_point_inside(ptr->aabb, (vec2){(float)i_state.pointer[0],
                                              (float)i_state.pointer[1]}));
  } else {
    return false;
  }
}

void gui_draggable(const char *name, AABB *aabb) {
  GUIDraggable *ptr = w_cm_return_slot(&(gui_state.draggables), name);

  if (ptr) {
    ptr->type = WT_DRAGGABLE;

    // new insertion.
    DEFAULT_Z_INDEX();
    gui_quad_render(&(ptr->render));
    memcpy(&(ptr->aabb), aabb, sizeof(AABB));
  } else {
    ptr = w_cm_get(&(gui_state.draggables), name);
  }

  if (i_state.act_held[ACT_HUD_INTERACT]) {
    if (is_point_inside(ptr->aabb, i_state.pointer)) {
      memcpy(&(ptr->aabb.center), i_state.pointer, sizeof(float) * 2);
    }
  }

  END_WIDGET();
}

#undef DEFAULT_Z_INDEX

inline static GUIWidget *gui_get_root() {
  return *(GUIWidget **)gui_state.window_stack.base_pointer;
}

// call gui functions after the gui update.
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
}

void gui_push() { gui_state.is_pushing = true; }

void gui_pop() {
  gui_state.last_added = w_stack_pop(&(gui_state.window_stack));
  NULL_CHECK(gui_state.last_added);
}

static mat4 model;
// after drawing and handling all the widget states, we need to clean the child
// arrays in each widget so that the next frame can be processed and setup
// seperately all over again. we'll recurse downward for the drawing/transform
// composition pass, and then once we reach the leaves, we'll recurse upward
// until we don't find another parent.
static void gui_clean_child_state_recursive(GUIWidget *ptr) {
  ptr->num_children = 0;
  if (ptr->parent) {
    gui_clean_child_state_recursive(ptr->parent);
  }
}

static float get_center_axis(AABB *to, AABB *by, int index) {
  float a = (by->center[index] - by->extents[index]);
  float b = (by->center[index] + by->extents[index]);
  float t = to->center[index];

  float lerp = ((1 - t) * a) + (t * b);

  PRINT_FLOAT(a);
  PRINT_FLOAT(b);
  PRINT_FLOAT(t);
  PRINT_FLOAT(lerp);

  return lerp;
}

static float get_extent_axis(AABB *to, AABB *by, int index) {
  float new_extent = (by->extents[index] * 2) * (to->extents[index]);
  PRINT_FLOAT(new_extent);
  return new_extent;
}

static void apply_transform(AABB *to, AABB *by, AABB *dest) {
  dest->center[0] = get_center_axis(to, by, 0);
  dest->center[1] = get_center_axis(to, by, 1);

  dest->extents[0] = get_extent_axis(to, by, 0);
  dest->extents[1] = get_extent_axis(to, by, 1);

#undef CENTER_AXIS
}

static void gui_draw_recursive(GUIWidget *ptr, AABB transform) {
#define CAST(T) T *widget = (T *)ptr;

#define APPLY_Z()                                                              \
  { glm_translate(model, (vec3){0, 0, widget->z_index}); }

#define TEXT_SQUISH()                                                          \
  {                                                                            \
    glm_translate(model, (vec3){temp.center[0], temp.center[1], 0});           \
    glm_scale(model, (vec3){(temp.extents[0] * 2) / widget->buf_len,           \
                            (temp.extents[1] * 2), 1});                        \
  }

#define DRAW()                                                                 \
  {                                                                            \
    shader_set_matrix4fv(gui_program, "u_model", (float *)model);              \
    glBindVertexArray(widget->render.vao);                                     \
    glDrawElements(GL_TRIANGLES, widget->render.n_idx, GL_UNSIGNED_INT, 0);    \
  }

  // wtf why do we need this? i thought passing the struct by value was enough
  // to get a proper data copy, but i guess not.
  AABB temp;
  // apply then recursively propagate the temp through the tree.
  apply_transform(&(ptr->aabb), &transform, &temp);
  print_vec4(&(temp), 0);

  glm_mat4_identity(model);

  switch (ptr->type) {
  case WT_WIDGET: {
  } break;
  case WT_LABEL: {
    // keep the u_model updated in a loop.
    CAST(GUILabel)
    TEXT_SQUISH()
    APPLY_Z()
    DRAW();
  } break;
  case WT_BUTTON: {
    CAST(GUIButton)
    TEXT_SQUISH()
    APPLY_Z()
    DRAW();
  } break;
  case WT_DRAGGABLE: {
    CAST(GUIDraggable);
    glm_translate(model, (vec3){temp.center[0], temp.center[1], 0});
    glm_scale(model, (vec3){temp.extents[0] * 2, temp.extents[1] * 2, 1});
    APPLY_Z()
    DRAW();
  } break;
  default: {
    char buf[256];
    sprintf(buf, "invalid widget type in the draw switch, found %d", ptr->type);
    ERROR_FROM_BUF(buf);
  } break;
  }

  int num_child_renders = ptr->num_children;

  // gui_draw_recursive will mutate the ptr->num_children, so we need to be
  // careful in saving all that data so that all children are drawn and iterated
  // through.
  for (int i = 0; i < num_child_renders; i++) {
    GUIWidget *child_ptr = ptr->children[i];
    gui_draw_recursive(child_ptr, temp);
  }

  if (ptr->num_children == 0) {
    // leaf node of the tree.
    gui_clean_child_state_recursive(ptr);
  }

#undef CAST
#undef APPLY_Z
#undef DRAW
#undef TEXT_SQUISH
}

void gui_draw() {
  mat4 model;
  glm_mat4_identity(model);

  shader_bind(gui_program);

  g_use_texture(gui_font->tex_handle, 0);

  GUIWidget *base_case = gui_get_root();
  RUNTIME_ASSERT(base_case->parent == NULL);

  gui_draw_recursive(base_case, base_case->aabb);
}

void gui_clean() {
  w_clean_array(&(gui_state.widgets));
  w_clean_array(&(gui_state.buttons));
  w_clean_array(&(gui_state.labels));
  w_clean_array(&(gui_state.draggables));

  w_stack_clean(&(gui_state.window_stack));
}
