#include "character.h"

#include "../object.h"
#include "animation/animator.h"
#include "backends/graphics_api.h"
#include "cglm/affine-pre.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "core/area_server.h"
#include "event_types.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "input_help.h"
#include "mathdef.h"
#include "object_lut.h"
#include "path.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Character entity type.

#define CAST Character *character = (Character *)p
#define CAST_RB RigidBody *rb = (RigidBody *)character->phys->body

Character *character_build(Model *model) {
  Character *p = (Character *)malloc(sizeof(Character));
  p->type = OBJ_CHARACTER;

  {
    p->phys = make_physcomp(
        (Body *)make_rigid_body(0.5, 0.9, 1.0, 0.9, 0.5, 0.5, 0.3, false,
                                (vec3){1, 0, 9}, 0.25, IDENTITY_VERSOR),
        (Collider *)make_sphere_collider(1.0));
  }

  { // characters can animate their own models.
    // parse then mesh the glb file, then render it in the normal drawing loop.
    p->render = make_rendercomp_from_glb(MODEL_PATH("wiggle.glb"));

    Model *player_model = (Model *)(p->render->data);
    p->animator = make_animator(player_model);

    anim_play(p->animator, "wiggle", true);
  }

  return p;
}

void character_init(void *p) {}

void character_update(void *p) {}

void character_clean(void *p) {
  Character *character = (Character *)p;
  free(character);
}
