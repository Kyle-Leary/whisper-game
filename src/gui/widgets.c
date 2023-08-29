#include "widgets.h"
#include "cglm/mat4.h"
#include "gui.h"

#include "gui/gui_renders.h"
#include "input/input.h"
#include "ogl_includes.h"
#include "path.h"
#include "render/texture.h"
#include "shaders/shader.h"
#include "whisper/colmap.h"
#include <sys/types.h>

static int default_z_index = 0;

static mat4 model;

uint draggable_texture;

#define CAST(T) T *widget = (T *)ptr;

#define APPLY_Z()                                                              \
  { glm_translate(model, (vec3){0, 0, widget->z_index}); }

#define TEXT_SQUISH()                                                          \
  {                                                                            \
    glm_translate(model, (vec3){ptr->global_aabb.center[0],                    \
                                ptr->global_aabb.center[1], 0});               \
    glm_scale(model,                                                           \
              (vec3){(ptr->global_aabb.extents[0] * 2) / widget->buf_len,      \
                     (ptr->global_aabb.extents[1] * 2), 1});                   \
  }

#define DRAW()                                                                 \
  {                                                                            \
    shader_set_matrix4fv(render_state.shader, "u_model", (float *)model);      \
    glBindVertexArray(widget->render.vao);                                     \
    glDrawElements(GL_TRIANGLES, widget->render.n_idx, GL_UNSIGNED_INT, 0);    \
    glm_mat4_identity(model);                                                  \
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
    gui_internal_add_child((GUIWidget *)ptr);                                  \
    if (gui_state.is_pushing) {                                                \
      gui_internal_push((GUIWidget *)ptr);                                     \
    }                                                                          \
  }

#define USE_TEXTURE(tex)                                                       \
  { g_use_texture(tex, 0); }

static void widget_draw(GUIWidget *ptr) {}
static void widget_update(GUIWidget *ptr, GUIInputState *input_state) {}

void gui_widget(const char *name) {
  GUIWidget *ptr = w_cm_return_slot(&(gui_state.widgets), name);

  if (ptr) {
    // first time init
    ptr->type = WT_WIDGET;
    DEFAULT_Z_INDEX();
  } else {
    // else, it's returned NULL and we need to grab the slot ourselves.
    ptr = w_cm_get(&(gui_state.widgets), name);
  }

  END_WIDGET();
}

static void label_draw(GUIWidget *ptr) {
  CAST(GUILabel);
  TEXT_SQUISH();
  APPLY_Z();
  USE_TEXTURE(render_state.font->tex_handle);
  DRAW();
}

static void label_update(GUIWidget *ptr, GUIInputState *input_state) {}

void gui_label(const char *name, const char *text) {
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

static void button_draw(GUIWidget *ptr) {
  CAST(GUIButton)
  TEXT_SQUISH()
  APPLY_Z()
  USE_TEXTURE(render_state.font->tex_handle);
  DRAW();
}

static void button_update(GUIWidget *ptr, GUIInputState *input_state) {
  GUIButton *button = (GUIButton *)ptr;
  if (input_state->mouse_button_just_clicked[GUI_MOUSE_LEFT] &&
      input_state->mouse_inside) {
    button->is_clicked = true;
  } else {
    button->is_clicked = false;
  }
}

bool gui_button(const char *name, const char *text) {
  GUIButton *ptr = w_cm_return_slot(&(gui_state.buttons), name);

  if (ptr) {
    ptr->type = WT_BUTTON;

    // new insertion.
    gui_string_render(&(ptr->render), text);

    int len = strlen(text);
    memcpy(ptr->buffer, text, len);
    DEFAULT_Z_INDEX();
    ptr->buf_len = len;

  } else {

    ptr = w_cm_get(&(gui_state.buttons), name);
  }

  END_WIDGET();

  return ptr->is_clicked;
}

static void draggable_draw(GUIWidget *ptr) {
  CAST(GUIDraggable);
  glm_translate(
      model, (vec3){ptr->global_aabb.center[0], ptr->global_aabb.center[1], 0});
  glm_scale(model, (vec3){ptr->global_aabb.extents[0] * 2,
                          ptr->global_aabb.extents[1] * 2, 1});
  APPLY_Z();
  USE_TEXTURE(draggable_texture);
  DRAW();
}

static void draggable_update(GUIWidget *ptr, GUIInputState *input_state) {
  if (input_state->mouse_button[GUI_MOUSE_LEFT] && input_state->mouse_inside) {
    memcpy(&(ptr->aabb.center), i_state.pointer, sizeof(float) * 2);
  }
}

void gui_draggable(const char *name) {
  GUIDraggable *ptr = w_cm_return_slot(&(gui_state.draggables), name);

  if (ptr) {
    ptr->type = WT_DRAGGABLE;

    // new insertion.
    DEFAULT_Z_INDEX();
    gui_quad_render(&(ptr->render));
  } else {
    ptr = w_cm_get(&(gui_state.draggables), name);
  }

  END_WIDGET();
}

WidgetHandler widget_handlers[WT_COUNT] = {
    [WT_WIDGET] =
        {
            widget_draw,
            widget_update,
        },
    [WT_LABEL] =
        {
            label_draw,
            label_update,
        },
    [WT_BUTTON] =
        {
            button_draw,
            button_update,
        },
    [WT_DRAGGABLE] =
        {
            draggable_draw,
            draggable_update,
        },
};

void gui_widget_types_init() {
  glm_mat4_identity(model);

  draggable_texture = g_load_texture(TEXTURE_PATH("draggable.png"));

  w_create_cm(&(gui_state.widgets), sizeof(GUIWidget), 256);
  w_create_cm(&(gui_state.labels), sizeof(GUILabel), 256);
  w_create_cm(&(gui_state.buttons), sizeof(GUIButton), 256);
  w_create_cm(&(gui_state.draggables), sizeof(GUIDraggable), 256);
}

#undef CAST
#undef APPLY_Z
#undef DRAW
#undef TEXT_SQUISH

#undef DEFAULT_Z_INDEX
