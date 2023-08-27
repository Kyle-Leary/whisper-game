#include "gui.h"
#include "cglm/affine.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "helper_math.h"
#include "input/input.h"
#include "meshing/font.h"
#include "path.h"
#include "render/graphics_render.h"
#include "render/render_configuration.h"
#include "shaders/shader.h"
#include "shaders/shader_binding.h"
#include "whisper/colmap.h"
#include "whisper/hashmap.h"
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

// highest z_index is on the top.
#define WIDGET_COMMON                                                          \
  GraphicsRender *render;                                                      \
  bool is_in_use;                                                              \
  int z_index;

typedef struct GUILabel {
  WIDGET_COMMON
  char buffer[256];
  int buf_len;
  AABB aabb;
} GUILabel;

// just a draggable quad. this always has 6 indices.
typedef struct GUIDraggable {
  WIDGET_COMMON
  AABB aabb;
} GUIDraggable;

typedef struct GUIButton {
  WIDGET_COMMON
  char buffer[256];
  int buf_len;
  AABB aabb;
} GUIButton;

typedef struct GUIState {
  WColMap labels;
  WColMap buttons;
  WColMap draggables;
} GUIState;

static GUIState gui_state = {0};

static int default_z_index = 0;

// execute "block" with the widget context ptr "w".
#define WIDGET_MAP_FORALL(widget_t, widget_colmap, block)                      \
  {                                                                            \
    WColMap *cm = &(gui_state.widget_colmap);                                  \
    for (int i = 0; i < cm->num_elms; i++) {                                   \
      widget_t *w = w_array_get(cm, i);                                        \
      if (w) {                                                                 \
        block                                                                  \
      }                                                                        \
    }                                                                          \
  }

// wrapper macro with all the widget types defined.
#define ALL_WIDGETS_DO(block)                                                  \
  {                                                                            \
    WIDGET_MAP_FORALL(GUILabel, labels, block);                                \
    WIDGET_MAP_FORALL(GUIButton, buttons, block);                              \
  }

static Font *gui_font;
static Shader *gui_program;
static mat4 g_projection;

void gui_init() {
  w_create_cm(&(gui_state.labels), sizeof(GUILabel), 256);
  w_create_cm(&(gui_state.buttons), sizeof(GUIButton), 256);
  w_create_cm(&(gui_state.draggables), sizeof(GUIDraggable), 256);

  gui_font =
      font_init(16, 16, textures[g_load_texture(TEXTURE_PATH("ui_font.png"))]);

  { // init mats/global shader ref from the shader subsystem.
    gui_program = get_shader("gui");
    glm_ortho(0, 1, 0, 1, 0.01, 100, g_projection);

    shader_bind(gui_program);
    shader_set_matrix4fv(gui_program, "u_projection", (float *)g_projection);
  }
}

static GraphicsRender *
gui_make_render(float *positions, float *uvs, unsigned int *indices,
                uint num_indices,
                uint num_verts) { // init test render in opengl.
  return g_new_render(
      (VertexData *)&(HUDVertexData){RC_HUD, num_verts, positions, uvs},
      indices, num_indices);
}

static GraphicsRender *gui_quad_render() {
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

  return gui_make_render(positions, uvs, indices, 6, 4);
}

static GraphicsRender *gui_string_render(const char *str) { // init test render.
  uint len = strlen(str);

  int num_verts = len * 4;
  int num_indices = len * 6;

  float positions[num_verts * 2];
  float uvs[num_verts * 2];
  uint indices[num_indices];

  font_mesh_string_raw(gui_font, str, len, 1, 1, positions, uvs, indices);

  return gui_make_render(positions, uvs, indices, num_indices, num_verts);
}

#define DEFAULT_Z_INDEX()                                                      \
  {                                                                            \
    ptr->z_index = default_z_index;                                            \
    default_z_index += 5;                                                      \
  }

void gui_label(const char *text, AABB *aabb) {
  GUILabel l;
  void *label_ptr = w_cm_insert(&(gui_state.labels), text, &l);
  if (label_ptr) {
    GUILabel *ptr = (GUILabel *)label_ptr;
    // new insertion.
    ptr->render = gui_string_render(text);
    int len = strlen(text);
    DEFAULT_Z_INDEX();
    memcpy(ptr->buffer, text, len);
    ptr->buf_len = len;
    memcpy(&(ptr->aabb), aabb, sizeof(AABB));
    ptr->is_in_use = true;
  } else {
    GUILabel *ptr = w_cm_get(&(gui_state.labels), text);
    ptr->is_in_use = true;
  }
}

bool gui_button(const char *text, AABB *aabb) {
  GUIButton l;
  void *button_ptr = w_cm_insert(&(gui_state.buttons), text, &l);
  if (button_ptr) {
    GUIButton *ptr = (GUIButton *)button_ptr;
    // new insertion.
    ptr->render = gui_string_render(text);
    int len = strlen(text);
    memcpy(ptr->buffer, text, len);
    DEFAULT_Z_INDEX();
    ptr->buf_len = len;
    memcpy(&(ptr->aabb), aabb, sizeof(AABB));
    ptr->is_in_use = true;
  } else {
    GUIButton *ptr = w_cm_get(&(gui_state.buttons), text);
    ptr->is_in_use = true;

    if (i_state.act_just_pressed[ACT_HUD_INTERACT]) {
      return (is_point_inside(ptr->aabb, (vec2){(float)i_state.pointer[0],
                                                (float)i_state.pointer[1]}));
    }
  }

  return false;
}

void gui_draggable(const char *name, AABB *aabb) {
  GUIDraggable l;
  void *draggable_ptr = w_cm_insert(&(gui_state.draggables), name, &l);
  if (draggable_ptr) {
    GUIDraggable *ptr = (GUIDraggable *)draggable_ptr;
    // new insertion.
    DEFAULT_Z_INDEX();
    ptr->render = gui_quad_render();
    memcpy(&(ptr->aabb), aabb, sizeof(AABB));
    ptr->is_in_use = true;
  } else {
    GUIDraggable *ptr = w_cm_get(&(gui_state.draggables), name);
    ptr->is_in_use = true;
    if (i_state.act_held[ACT_HUD_INTERACT]) {
      if (is_point_inside(ptr->aabb, i_state.pointer)) {
        memcpy(&(ptr->aabb.center), i_state.pointer, sizeof(float) * 2);
      }
    }
  }
}

#undef DEFAULT_Z_INDEX

// call gui functions after the gui update.
void gui_update() {
  ALL_WIDGETS_DO({ w->is_in_use = false; })
}

void gui_draw() {
  shader_bind(gui_program);

  g_use_texture(gui_font->tex_handle, 0);

#define GENERIC_PREDRAW()                                                      \
  { glm_mat4_identity(w->render->model); }

#define APPLY_Z()                                                              \
  { glm_translate(w->render->model, (vec3){0, 0, w->z_index}); }

#define TEXT_SQUISH()                                                          \
  {                                                                            \
    glm_translate(w->render->model,                                            \
                  (vec3){w->aabb.center[0], w->aabb.center[1], 0});            \
    glm_scale(w->render->model, (vec3){(w->aabb.extents[0] * 2) / w->buf_len,  \
                                       (w->aabb.extents[1] * 2), 1});          \
  }

#define DRAW()                                                                 \
  { g_draw_render(w->render); }

  WIDGET_MAP_FORALL(GUILabel, labels, {
    if (w->is_in_use) {
      // keep the u_model updated in a loop.
      GENERIC_PREDRAW()
      TEXT_SQUISH()
      APPLY_Z()
      DRAW();
    }
  });

  WIDGET_MAP_FORALL(GUIButton, buttons, {
    if (w->is_in_use) {
      GENERIC_PREDRAW()
      TEXT_SQUISH()
      APPLY_Z()
      DRAW();
    }
  });

  WIDGET_MAP_FORALL(GUIDraggable, draggables, {
    if (w->is_in_use) {
      GENERIC_PREDRAW()
      glm_translate(w->render->model,
                    (vec3){w->aabb.center[0], w->aabb.center[1], 0});
      glm_scale(w->render->model,
                (vec3){w->aabb.extents[0], w->aabb.extents[1], 1});
      APPLY_Z()
      DRAW();
    }
  });

#undef TEXT_SQUISH
}

void gui_clean() {}
