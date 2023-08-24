#include "sphere.h"

#include "../object.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "physics/detection.h"
#include "render.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Sphere entity type.

#define CAST Sphere *sphere = (Sphere *)p
#define CAST_RB RigidBody *rb = (RigidBody *)sphere->phys->body

Sphere *sphere_build(vec3 position, float radius, unsigned int segments) {
  Sphere *p = (Sphere *)calloc(sizeof(Sphere), 1);
  p->type = OBJ_SPHERE;

  {
    p->phys = make_physcomp(
        (Body *)make_rigid_body(0.9, 1.0, 0.9, 0.5, 0.3, true, (vec3){1, 0, 9}),
        (Collider *)make_sphere_collider(radius));
  }

  {
    p->render = make_rendercomp_from_graphicsrender(
        glprim_sphere(p->phys->body->position, radius, segments));
  }

  return p;
}

void sphere_init(void *p) {}

void sphere_update(void *p) {
  CAST;
  CAST_RB;

  GraphicsRender *prim = sphere->render->data;
  glm_mat4_identity(prim->model);
  glm_translate(prim->model, rb->lerp_position);
  g_draw_render(prim);

  { // handle physevents
    WQueue mailbox = sphere->phys->collider->phys_events;
    while (mailbox.active_elements > 0) {
      CollisionEvent *e = w_dequeue(&mailbox);
      assert(
          e !=
          NULL); // shouldn't happen with the active elements condition above.
    }
  }
}

void sphere_clean(void *p) {
  Sphere *sphere = (Sphere *)p;
  free(sphere);
}
