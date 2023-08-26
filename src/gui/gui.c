#include "gui.h"
#include "backends/graphics/shader.h"
#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "cglm/affine.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "helper_math.h"
#include "meshing/font.h"
#include "path.h"
#include "whisper/colmap.h"
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

// highest z_index is on the top.
#define WIDGET_COMMON                                                          \
  bool is_in_use;                                                              \
  int z_index;

typedef struct GUILabel {
  WIDGET_COMMON
  char buffer[256];
  int buf_len;
  AABB aabb;
  uint vao;
  uint n_idx;
} GUILabel;

// just a draggable quad. this always has 6 indices.
typedef struct GUIDraggable {
  WIDGET_COMMON
  AABB aabb;
  uint vao;
} GUIDraggable;

typedef struct GUIButton {
  WIDGET_COMMON
  char buffer[256];
  int buf_len;
  AABB aabb;
  uint vao;
  uint n_idx;
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

  { // init mats
    glm_ortho(0, 1, 0, 1, 0.01, 100, g_projection);
  }

  // init shader w/ uniforms
  {
    gui_program = make_shader(SHADER_PATH("gui.vs"), SHADER_PATH("gui.fs"));
    shader_set_1i(gui_program, "u_tex_sampler", 0); // 0th slot
    // fix the projection at shader-creation time.
    shader_set_matrix4fv(gui_program, "u_projection", (float *)g_projection);
  }
}

static void gui_make_vao(uint *vao, float *positions, float *uvs,
                         unsigned int *indices, uint num_indices,
                         uint num_verts) { // init test render in opengl.
  glGenVertexArrays(1, &(*vao));
  glBindVertexArray(*vao);

  GLuint vbo_pos_id;
  glGenBuffers(1, &vbo_pos_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_verts, positions,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        (GLvoid *)0);
  glEnableVertexAttribArray(0);

  GLuint vbo_uv_id;
  glGenBuffers(1, &vbo_uv_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_uv_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_verts, uvs,
               GL_STATIC_DRAW);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        (GLvoid *)0);
  glEnableVertexAttribArray(1);

  GLuint ebo_id;
  glGenBuffers(1, &ebo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * num_indices, indices,
               GL_STATIC_DRAW);
}

static void gui_quad_vao(uint *vao) {
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

  gui_make_vao(vao, positions, uvs, indices, 6, 4);
}

static void gui_string_vao(uint *vao, uint *n_idx,
                           const char *str) { // init test render.
  uint len = strlen(str);

  int num_verts = len * 4;
  int num_indices = len * 6;

  float positions[num_verts * 2];
  float uvs[num_verts * 2];
  uint indices[num_indices];

  *n_idx = num_indices;

  font_mesh_string_raw(gui_font, str, len, 1, 1, positions, uvs, indices);

  { // init test render in opengl.
    glGenVertexArrays(1, &(*vao));
    glBindVertexArray(*vao);

    GLuint vbo_pos_id;
    glGenBuffers(1, &vbo_pos_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_verts, positions,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);

    GLuint vbo_uv_id;
    glGenBuffers(1, &vbo_uv_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_uv_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_verts, uvs,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (GLvoid *)0);
    glEnableVertexAttribArray(1);

    GLuint ebo_id;
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * num_indices, indices,
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
  }
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
    gui_string_vao(&(ptr->vao), &(ptr->n_idx), text);
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
    gui_string_vao(&(ptr->vao), &(ptr->n_idx), text);
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
    gui_quad_vao(&(ptr->vao));
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
  mat4 model;
  glm_mat4_identity(model);

  g_use_texture(gui_font->tex_handle, 0);

#define GENERIC_PREDRAW()                                                      \
  { glm_mat4_identity(model); }

#define APPLY_Z()                                                              \
  { glm_translate(model, (vec3){0, 0, w->z_index}); }

#define TEXT_SQUISH()                                                          \
  {                                                                            \
    glm_translate(model, (vec3){w->aabb.center[0], w->aabb.center[1], 0});     \
    glm_scale(model, (vec3){(w->aabb.extents[0] * 2) / w->buf_len,             \
                            (w->aabb.extents[1] * 2), 1});                     \
  }

#define APPLY_MODEL()                                                          \
  { shader_set_matrix4fv(gui_program, "u_model", (float *)model); }

  WIDGET_MAP_FORALL(GUILabel, labels, {
    if (w->is_in_use) {
      // keep the u_model updated in a loop.
      GENERIC_PREDRAW()
      TEXT_SQUISH()
      APPLY_Z()
      APPLY_MODEL()
      glBindVertexArray(w->vao);
      glDrawElements(GL_TRIANGLES, w->n_idx, GL_UNSIGNED_INT, 0);
    }
  });

  WIDGET_MAP_FORALL(GUIButton, buttons, {
    if (w->is_in_use) {
      GENERIC_PREDRAW()
      TEXT_SQUISH()
      APPLY_Z()
      APPLY_MODEL()
      glBindVertexArray(w->vao);
      glDrawElements(GL_TRIANGLES, w->n_idx, GL_UNSIGNED_INT, 0);
    }
  });

  WIDGET_MAP_FORALL(GUIDraggable, draggables, {
    if (w->is_in_use) {
      GENERIC_PREDRAW()
      glm_translate(model, (vec3){w->aabb.center[0], w->aabb.center[1], 0});
      glm_scale(model, (vec3){w->aabb.extents[0], w->aabb.extents[1], 1});
      APPLY_Z()
      APPLY_MODEL()
      glBindVertexArray(w->vao);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
  });

#undef TEXT_SQUISH
}

void gui_clean() {}
