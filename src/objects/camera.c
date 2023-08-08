#include "camera.h"

#include "../cglm/types.h"
#include "../cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic camera entity type.

#define CAST Camera *camera = (Camera *)p

Camera *camera_build(vec3 position, vec3 *target) {
  Camera *p = (Camera *)malloc(sizeof(Camera));
  memcpy(p->position, position, sizeof(float) * 3);
  p->num_colliders = 1;
  p->colliders = (Collider *)malloc(
      sizeof(Collider) * p->num_colliders); // space for one collider.
  p->colliders[0].type = CL_PILLAR;
  p->target = target;
  p->type = OBJ_CAMERA;
  return p;
}

void camera_init(void *p) {}

void camera_update(void *p) {
  CAST;

  { // calc the camera position from the target
    // TODO: lerping and rotation with the mouse?
    vec3 offset;
    glm_vec3_one(offset);
    glm_vec3_scale(
        offset, 5,
        offset); // offset by {5, 5, 5} vector from the target for now.
    glm_vec3_add(*camera->target, offset, camera->position);
  }

  { // then, update the view matrix with the new position.
    mat4 offset_m;
    glm_mat4_identity(offset_m);
    glm_translate(offset_m, camera->position);

    glm_lookat((vec3){0}, *camera->target, (vec3){0, 0.9, 0.01}, m_view_tf);
  }
}

void camera_handle_collision(void *p, CollisionEvent *e) {
  CAST;
  glm_vec3_scale(e->normalized_force, e->magnitude, e->normalized_force);
  glm_vec3_add(camera->position, e->normalized_force, camera->position);
}

void camera_clean(void *p) {
  CAST;
  free(camera);
}
