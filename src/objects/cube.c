#include "cube.h"

#include "../object.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"

#include "helper_math.h"
#include "input_help.h"

#include "mathdef.h"
#include "physics/body/body.h"
#include "physics/collider/collider.h"
#include "printers.h"

#include "render/gr_prim.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Cube entity type.

#define CAST Cube *cube = (Cube *)p
#define CAST_RB RigidBody *rb = (RigidBody *)cube->phys->body

Cube *cube_build(vec3 position, vec3 extents) {
  Cube *p = (Cube *)malloc(sizeof(Cube));

  p->type = OBJ_CUBE;

  {
    p->phys = make_physcomp((Body *)make_rigid_body(0.5, 0.1, 1.0, 0.5, 0.9,
                                                    0.5, 0.3, true, position,
                                                    1.0, IDENTITY_VERSOR),
                            (Collider *)make_rect_collider(extents));
  }

  p->phys->body->rotation[0] += 1;
  p->phys->body->rotation[1] += 0.5;

  p->render = make_rendercomp_from_graphicsrender(gr_prim_rect(extents));

  return p;
}

void cube_init(void *p) {}

void cube_update(void *p) {
  CAST;
  CAST_RB;

  rb->ang_acceleration[0] = 5;

  GraphicsRender *prim = cube->render->data;
  glm_mat4_identity(prim->model);
  glm_translate(prim->model, rb->lerp_position);

  { // apply rotation from the versor:
    // make the rotation matrix from the versor
    mat4 rot_mat;

    glm_quat_mat4(rb->rotation, rot_mat);
    glm_mat4_mul(prim->model, rot_mat, prim->model);
  }

  g_draw_render(prim);
}

void cube_clean(void *p) {
  Cube *cube = (Cube *)p;
  free(cube);
}
