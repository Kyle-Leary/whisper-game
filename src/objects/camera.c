#include "camera.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "backends/input_api.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "physics/raycast.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic camera entity type.

#define CAST Camera *camera = (Camera *)p

Camera *camera_build(vec3 position, vec3 *target) {
  Camera *p = (Camera *)malloc(sizeof(Camera));

  { // give the camera basic collisions with the world, this will be useful when
    // trying to make it not jank/clipping through walls.
    // Collider *colliders = (Collider *)calloc(sizeof(Collider), 1);
    // make_colliders(1, colliders);
    //
    // colliders[0].type = CL_SPHERE;
    // SphereColliderData *col_data =
    //     (SphereColliderData *)malloc(sizeof(SphereColliderData) * 1);
    // col_data->radius = 1;
    // colliders[0].data = col_data;

    p->phys =
        make_physcomp(0.1, 1.0, 0.5, true, true, NULL, 1, position, false);
  }

  p->target = target;
  p->rotation = 0;
  p->rotation_speed = 0.01;

  p->height = 5;
  p->rising_speed = 0.1;

  p->type = OBJ_CAMERA;
  return p;
}

void camera_init(void *p) {}

#define MAX_MOUSEPICKING_OBJECTS 3

// handle mouse raycasting and clicking.
static void mouse_picking_loop(Camera *camera) {
  // if we're clicking, send out a raycast and inform all the objects we've hit.
  if (i_state.act_just_pressed[ACT_HUD_INTERACT]) {
    uint16_t indices[MAX_MOUSEPICKING_OBJECTS];

    int num_found =
        raycast_intersect(indices, MAX_MOUSEPICKING_OBJECTS,
                          camera->phys->lerp_position, (vec3){0, -1, 0});

    for (int i = 0; i < num_found; i++) {
      uint16_t id = indices[i];
      printf("intersected with oid %d in the raycast.\n", id);
    }
  }
}

void camera_update(void *p) {
  CAST;

  mouse_picking_loop(camera);

  if (i_state.act_held[ACT_CAMERA_CW]) {
    camera->rotation += camera->rotation_speed;
  } else if (i_state.act_held[ACT_CAMERA_CCW]) {
    camera->rotation -= camera->rotation_speed;
  }

  if (i_state.act_held[ACT_CAMERA_RAISE]) {
    camera->height += camera->rising_speed;
  } else if (i_state.act_held[ACT_CAMERA_LOWER]) {
    camera->height -= camera->rising_speed;
  }

  camera->height = glm_clamp(camera->height, 1, 20);

  { // calc the camera position from the target
    // TODO: lerping and rotation with the mouse?
    vec3 offset;
    memcpy(offset,
           (vec3){13 * sin(camera->rotation), camera->height,
                  13 * cos(camera->rotation)},
           sizeof(float) * 3);
    glm_vec3_add(*camera->target, offset, camera->phys->position);
  }

  { // then, update the view matrix with the new position.
    mat4 offset_m;
    glm_mat4_identity(offset_m);
    glm_translate(offset_m, camera->phys->position);

    glm_lookat(camera->phys->position, *camera->target, (vec3){0, 0.9, 0.01},
               m_view);
  }
}

void camera_handle_collision(void *p, CollisionEvent *e) {
  CAST;
  glm_vec3_scale(e->normalized_force, e->magnitude, e->normalized_force);
  glm_vec3_add(camera->phys->position, e->normalized_force,
               camera->phys->position);
}

void camera_clean(void *p) {
  CAST;
  free(camera);
}
