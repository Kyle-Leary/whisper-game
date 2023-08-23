#include "character.h"

#include "../object.h"
#include "../physics.h"
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
#include "object_lut.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// really basic Character entity type.

#define CAST Character *character = (Character *)p

Character *character_build(Model *model) {
  Character *p = (Character *)malloc(sizeof(Character));
  p->type = OBJ_CHARACTER;

  {
    Collider *colliders = (Collider *)calloc(sizeof(Collider), 1);
    make_colliders(1, colliders);
    colliders[0].type = CL_SPHERE;
    SphereColliderData *col_data =
        (SphereColliderData *)malloc(sizeof(SphereColliderData) * 1);
    col_data->radius = 1;
    colliders[0].data = col_data;

    p->phys = make_physcomp(0.1, 1.0, 0.5, 0.5, 0.3, false, false, colliders, 1,
                            (vec3){0});
  }

  { // characters can animate their own models.
    // memset(&(p->animator), 0, sizeof(Animator));
    // p->animator.target = p->model;
  }

  p->model = model;

  return p;
}

void character_init(void *p) {}

void character_update(void *p) {}

void character_draw(void *p) {
  CAST;

  glm_mat4_identity(character->model->transform);
  glm_translate(character->model->transform, character->phys->lerp_position);

  g_draw_model(character->model);
}

InteractionResponse character_handle_interact(void *p, InteractionEvent e) {
  CAST;

  if (e.x_pos != character->phys->position[0] ||
      e.y_pos !=
          character->phys->position[1]) // if the spot they want to move isn't
                                        // the character's spot, return nothing,
                                        // since we don't care about it.
    return (InteractionResponse){IRT_NONE};

  // else, they're trying to move into our spot.
  switch (e.type) {
  case IT_MOVE_INTO:
    return (InteractionResponse){IRT_BLOCK}; // block them, don't let them move.
    break;
  case IT_EXAMINE:
    return (InteractionResponse){
        IRT_ENCOUNTER}; // block them, don't let them move.
    break;
  default:
    return (InteractionResponse){IRT_NONE};
    break;
  }
}

void character_clean(void *p) {
  Character *character = (Character *)p;
  free(character);
}
