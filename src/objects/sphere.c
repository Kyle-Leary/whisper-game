#include "sphere.h"

#include "../object.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "global.h"

#include "helper_math.h"
#include "im_prims.h"
#include "input_help.h"
#include "mathdef.h"
#include "physics/component.h"
#include "physics/detection.h"
#include "printers.h"
#include "render/gr_prim.h"
#include "render/render.h"
#include "transform.h"
#include "util.h"

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
    p->phys = make_physcomp((Body *)make_rigid_body(0.3, 0.7, 5.0, 0.5, 0.1,
                                                    0.5, 0.3, true, position,
                                                    5.0, IDENTITY_VERSOR),
                            (Collider *)make_sphere_collider(radius));
  }

  {
    p->render = make_rendercomp_from_graphicsrender(
        gr_prim_sphere(p->phys->body->position, radius, segments));
    p->render->data.gr->shader = get_shader("fraglight");
  }

  return p;
}

void sphere_init(void *p) {}

void sphere_update(void *p) {
  CAST;
  CAST_RB;

  { // handle physevents
    WQueue mailbox = sphere->phys->collider->phys_events;
    while (mailbox.active_elements > 0) {
      CollisionEvent *e = w_dequeue(&mailbox);
      im_point(e->contact_pt);
    }
  }

  { // render setup
    GraphicsRender *prim = sphere->render->data.gr;
    m4_apply_transform_from_body(prim->model, sphere->phys->body);
  }

  im_velocity(rb);
  im_acceleration(rb);
}

void sphere_clean(void *p) {
  Sphere *sphere = (Sphere *)p;
  free_rendercomp(sphere->render);
  free_physcomp(sphere->phys);
  free(sphere);
}
