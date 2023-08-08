#include "character.h"

#include "cglm/types.h"
#include "cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "cglm/mat4.h"
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

Character *character_build(ivec2 position, TextureHandle handle) {
  Character *p = (Character *)malloc(sizeof(Character));
  memcpy(p->position, position, sizeof(float) * 2);
  p->num_colliders = 1;
  p->colliders = (Collider *)malloc(
      sizeof(Collider) * p->num_colliders); // space for one collider.
  p->colliders[0].type = CL_PILLAR;
  p->type = OBJ_CHARACTER;
  vec3 d;
  area_get_vec3_from_position(p->position, d);
  p->render = glprim_upright_plane(d);
  p->handle = handle;
  return p;
}

void character_init(void *p) {}

void character_update(void *p) {
  CAST;
  glm_mat4_identity(character->render->model);

  vec3 player_pos;
  area_get_player_vec3(player_pos);

  // vec3 eye = {-character->position[0], 0, -character->position[2]};
  // vec3 up = {0, 1, 0}; // Assuming up direction is positive y-axis
  //
  // glm_lookat(eye, player_pos, up,
  //            character->render
  //                ->r_model); // load the lookat matrix into the passed mat.

  vec3 d;
  area_get_vec3_from_position(character->position, d);
  glm_translate(character->render->model, d);
  glm_scale(character->render->model, (vec3){0.5F, 2, 0.5F});
}

void character_draw(void *p) {
  CAST;
  g_use_texture(character->handle);
  g_draw_render(character->render);
}

InteractionResponse character_handle_interact(void *p, InteractionEvent e) {
  CAST;

  printf("Current character position: (%d, %d)\n", character->position[0],
         character->position[1]);
  printf("Desired position: (%d, %d)\n", e.x_pos, e.y_pos);

  if (e.x_pos != character->position[0] ||
      e.y_pos !=
          character->position[1]) // if the spot they want to move isn't the
                                  // character's spot, return nothing, since we
                                  // don't care about it.
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
