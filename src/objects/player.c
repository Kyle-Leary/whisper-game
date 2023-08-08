#include "backends/graphics_api.h"
#include "camera.h"

#include "cglm/types.h"
#include "cglm/vec3.h"
#include "../object.h"
#include "../physics.h"
#include "backends/input_api.h"
#include "cglm/affine-pre.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/vec2.h"
#include "core/area_defines.h"
#include "core/area_server.h"
#include "core/battle.h"
#include "event_types.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "input_help.h"
#include "meshing/obj/obj_parse.h"
#include "object_bases.h"
#include "path.h"
#include "pragma.h"
#include "state.h"

#include <GL/gl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAST Player *player = (Player *)p

// use the dummy as the default entity, just for the initial setup of the object
// pipeline in the game.
Player *player_build() {
  Player *p = (Player *)calloc(sizeof(Player), 1);
  memcpy(p->position, (vec3){0, -1.5F, 0},
         sizeof(float) * 3); // copy the literal from the stack.
  memcpy(p->lerp_position, p->position,
         sizeof(float) * 3); // copy the literal from the stack.
  // no collision? at least not yet.
  p->type = OBJ_PLAYER;
  p->position_lerp_speed = 0.1F;

  p->mass = 1.0F;
  p->linear_damping = 0.5F;

  p->num_colliders = 1;
  p->colliders = (Collider *)calloc(sizeof(Collider), 1);
  p->colliders[0].type = CL_SPHERE;

  SphereColliderData *col_data =
      (SphereColliderData *)malloc(sizeof(SphereColliderData) * 1);
  col_data->radius = 1;
  p->colliders[0].data = col_data;

  p->render = render_from_obj(MODEL_PATH("character.obj"));

  p->forward_speed = 1.0F;
  return p;
}

void player_init(void *p) {}

// Update player's mats and movement
static void update_player_mats_movement(Player *player) {}

static void player_handle_interactions(Player *player) {
  Position p;

  // compare the interaction point with the new testing position.
  if (i_state.act_just_pressed[ACT_WORLD_INTERACT]) {
    InteractionResponse responses[NUM_OBJECTS] = {0};
    area_interact((InteractionEvent){IT_EXAMINE, p[0], p[1]}, responses);

    for (int i = 0; i < NUM_OBJECTS; i++) {
      InteractionResponse r = responses[i];
      switch (r.type) {
      case IRT_ENCOUNTER:
        // then accept the encounter, and start it up in the state machine.
        state_change(GS_ENCOUNTER);
        break;
      default:
        break;
      }
    }
  }
}

// The main function
static void player_handle_walking_state(Player *player) {
  player_handle_interactions(player);

  {
    mat4 offset_m;
    glm_mat4_identity(offset_m);
    glm_translate(offset_m, player->lerp_position);

    vec2 v_move;
    get_movement_vec(v_move, i_state.act_held);

    if (fabs(v_move[0]) < 0.001F && fabs(v_move[1]) < 0.001F) {
      // no movement, don't bother
    } else {
      glm_vec2_scale(v_move, player->forward_speed, v_move);

      Basis b = get_basis(m_view_tf);

      b.forward[1] = 0; // don't move vertically for now.
      b.right[1] = 0;

      { // then, generate the player's force push and apply it.
        vec3 force_direction = {0};

        glm_vec3_scale(b.right, v_move[0], b.right);
        glm_vec3_add(force_direction, b.right, force_direction);
        glm_vec3_scale(b.forward, v_move[1], b.forward);
        glm_vec3_add(force_direction, b.forward, force_direction);

        physics_apply_force((PhysicsObject *)player, force_direction);
      }

      // then, take ANOTHER step, and use that to calculate the lookat matrix
      // for the proper model rotation. lazy!!!
      glm_vec3_copy(player->position, player->ghost_step);

      glm_vec3_add(player->ghost_step, b.right, player->ghost_step);
      glm_vec3_add(player->ghost_step, b.forward, player->ghost_step);
    }
  }
}

static void handle_encounter_swing(Player *player) {
  if (BETWEEN(i_state.pointer[0], 0.3, 0.7)) {
    // we're swinging around the middle of the screen, check for damage.
    float magnitude = glm_vec2_distance(
        (vec2){0, 0},
        i_state.pointer_velocity); // compare with zero for length check
    battle_attack_enemy(magnitude);
  }
}

static void player_handle_encounter(Player *player) {
  if (i_state.act_held[ACT_IS_SWINGING]) {
    handle_encounter_swing(player);
  }
}

void player_update(void *p) {
  // trust the caller themselves to pass the right type.
  CAST;

  // player->position[1] -= 0.01F;

  switch (game_state) {
  case GS_WALKING:
    player_handle_walking_state(player);
    break;
  case GS_ENCOUNTER:
    player_handle_encounter(player);
    break;
  default:
    break;
  }
}

void player_draw(void *p) {
  CAST;
  player->ghost_step[1] = player->lerp_position[1]; // make the lookahead flat,
                                                    // or else it looks weird.
  glm_lookat(player->lerp_position, player->ghost_step, (vec3){0, 1, 0},
             player->render->model);
  glm_mat4_inv(player->render->model, player->render->model);
  g_draw_render(player->render);
}

void player_handle_collision(void *p, CollisionEvent *e) {}

// maybe don't have a seperate destructor? is there a point to that?
void player_clean(void *p) {
  Player *player = (Player *)p;
  free(player);
}
