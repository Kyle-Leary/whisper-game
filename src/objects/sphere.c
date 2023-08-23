#include "sphere.h"

#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"
#include "glprim.h"

#include "helper_math.h"
#include "input_help.h"
#include "render.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Sphere entity type.

#define CAST Sphere *sphere = (Sphere *)p

Sphere *sphere_build(vec3 position, float radius, unsigned int segments) {
  Sphere *p = (Sphere *)calloc(sizeof(Sphere), 1);
  p->type = OBJ_SPHERE;

  {
    Collider *colliders = (Collider *)calloc(sizeof(Collider), 1);
    colliders[0].type = CL_SPHERE;
    SphereColliderData *col_data =
        (SphereColliderData *)malloc(sizeof(SphereColliderData) * 1);
    col_data->radius = radius;
    colliders[0].data = col_data;
    colliders[0].intangible = false;

    p->phys = make_physcomp(0.9, 2.0, 0.1, 0.5, 0.3, true, false, colliders, 1,
                            position);
  }

  {
    p->render = make_rendercomp_from_graphicsrender(
        glprim_sphere(p->phys->position, radius, segments));
  }

  return p;
}

void sphere_init(void *p) {}

void sphere_update(void *p) {
  CAST;
  GraphicsRender *prim = sphere->render->data;
  glm_mat4_identity(prim->model);
  glm_translate(prim->model, sphere->phys->lerp_position);
  g_draw_render(prim);

  { // handle physevents
    WQueue mailbox = sphere->phys->colliders[0].phys_events;
    while (mailbox.active_elements > 0) {
      PhysicsEvent *e = w_dequeue(&mailbox);
      assert(
          e !=
          NULL); // shouldn't happen with the active elements condition above.
      if (e->sender_col_type == CL_FLOOR) {
        printf("floor\n");
      }
    }
  }
}

void sphere_handle_collision(void *p, CollisionEvent *e) {}

void sphere_clean(void *p) {
  Sphere *sphere = (Sphere *)p;
  free(sphere);
}
