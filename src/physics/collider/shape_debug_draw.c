#include "shape_debug_draw.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "macros.h"
#include "physics/collider/collider.h"
#include "physics/physics.h"
#include "render/graphics_render.h"
#include "shaders/shader_instances.h"
#include "transform.h"
#include "util.h"
#include "whisper/array.h"

#include "render/gr_prim.h"

#include <stddef.h>

static Shader *wireframe_program;

void debug_shape_init() { wireframe_program = get_shader("wireframe"); }

// define a hashtable over a bunch of pointers, the pointers to the collision
// objects. we want to map each collision object to one GraphicsRender*, for the
// purposes of reusing the same graphical representation VAOs.
#define DEBUG_SHAPE_HASHTABLE_LEN 2048

static GraphicsRender *shape_renders[DEBUG_SHAPE_HASHTABLE_LEN] = {0};

static unsigned int pointer_hash(void *key) {
  // this is good enough for now? i guess?
  return (unsigned long)key % DEBUG_SHAPE_HASHTABLE_LEN;
}

static void update_floor_render(Collider *c, GraphicsRender *gr) {
  { // position the render
    glm_mat4_identity(gr->model);
    glm_translate(gr->model, c->body->position);
  }

  glm_scale(gr->model, (vec3){50, 50, 50});
}
static void update_rect_render(Collider *c, GraphicsRender *gr) {
  glm_mat4_identity(gr->model);
  glm_translate(gr->model, c->body->position);

  { // apply rotation from the versor:
    // make the rotation matrix from the versor
    mat4 rot_mat;

    glm_quat_mat4(c->body->rotation, rot_mat);
    glm_mat4_mul(gr->model, rot_mat, gr->model);
  }
}
static void update_sphere_render(Collider *c, GraphicsRender *gr) {
  m4_apply_transform_from_body(gr->model, c->body);
}

static GraphicsRender *create_floor_render(Collider *c) {
  GraphicsRender *g = gr_prim_floor_plane(c->body->position);
  g->shader = wireframe_program;
  return g;
}
static GraphicsRender *create_rect_render(Collider *c) {
  GraphicsRender *g = gr_prim_rect(((RectCollider *)c)->extents);
  g->shader = wireframe_program;
  return g;
}
static GraphicsRender *create_sphere_render(Collider *c) {
  SphereCollider *sc = (SphereCollider *)c;
  GraphicsRender *g = gr_prim_sphere(c->body->position, sc->radius, 7);
  g->shader = wireframe_program;
  return g;
}

#define DEBUG_SHAPE_PASS(shape, array)                                         \
  for (int i = 0; i < physics_state.array.upper_bound; i++) {                  \
    Collider *base_collider = w_array_get(&(physics_state.array), i);          \
    if (!base_collider) {                                                      \
      return;                                                                  \
    }                                                                          \
    int hashed = pointer_hash(base_collider);                                  \
    GraphicsRender *collider_render = shape_renders[hashed];                   \
    if (collider_render) {                                                     \
      update_##shape##_render(base_collider, collider_render);                 \
    } else {                                                                   \
      GraphicsRender *gr = create_##shape##_render(base_collider);             \
      if (gr) {                                                                \
        shape_renders[hashed] = gr;                                            \
      }                                                                        \
    }                                                                          \
  }

// iterate through all the shape arrays, either make debug renders for each or
// maintain them, keep their models and etc up to date.
void debug_shape_maintenance_pass() {
  DEBUG_SHAPE_PASS(floor, floors)
  DEBUG_SHAPE_PASS(sphere, spheres)
  DEBUG_SHAPE_PASS(rect, rects)
}

// draw all of the pre-generated shapes in the generation pass.
void debug_shape_draw() {
  for (int i = 0; i < DEBUG_SHAPE_HASHTABLE_LEN; i++) {
    GraphicsRender *collider_render = shape_renders[i];

    if (collider_render) {
      g_draw_render(collider_render);
    }
  }
}
