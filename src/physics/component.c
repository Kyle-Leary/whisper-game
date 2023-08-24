#include "component.h"
#include "physics/constructor_macros.h"
#include "physics/physics.h"
#include "whisper/array.h"

#include <stddef.h>
#include <stdio.h>

PhysComp *make_physcomp(Body *body, Collider *collider) {
  // do the normal constructor stuff
  PhysComp comp;
  comp.body = body;
  comp.collider = collider;

  // then, link the collider's position with the body's position.
  comp.collider->body = body;

  INDEX_AND_RETURN(comp, phys_comps)
}
