#include "mathdef.h"
#include "physics/body/body.h"
#include "physics/collider/collider.h"
#include "physics/physics.h"
#include <assert.h>

void test_physmacros() {
  physics_init();

  RigidBody *rb = make_rigid_body(0.7, 0, 0, 0.5, 0.5, 0.5, 0.3, false,
                                  (vec3){5, 0, 2}, 1, IDENTITY_VERSOR);
  StaticBody *sb = make_static_body(5, UNIT_X_VEC3, 8, IDENTITY_VERSOR);

  SphereCollider *sphere = make_sphere_collider(5);
  FloorCollider *floor = make_floor_collider();

  assert(IS_RB(rb));
  assert(!IS_RB(sb));

  assert(IS_FLOOR(floor));
  assert(IS_SPHERE(sphere));

  physics_clean();
}
