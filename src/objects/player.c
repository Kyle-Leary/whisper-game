#include "animation/animator.h"
#include "backends/graphics_api.h"
#include "camera.h"

#include "../object.h"
#include "../physics.h"
#include "backends/input_api.h"
#include "cglm/affine-pre.h"
#include "cglm/affine.h"
#include "cglm/affine2d.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "cglm/vec2.h"
#include "cglm/vec3.h"
#include "core/area_defines.h"
#include "core/area_server.h"
#include "core/battle.h"
#include "event.h"
#include "event_types.h"
#include "global.h"
#include "glprim.h"
#include "helper_math.h"
#include "input_help.h"
#include "meshing/gltf_mesher.h"
#include "meshing/obj/obj_parse.h"
#include "object_bases.h"
#include "parsers/gltf/gltf_parse.h"
#include "path.h"
#include "physics/collider_types.h"
#include "pragma.h"
#include "printers.h"
#include "render.h"
#include "state.h"
#include "util.h"
#include "whisper/queue.h"

#include <assert.h>
#include <printf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAST Player *player = (Player *)p

// use the dummy as the default entity, just for the initial setup of the object
// pipeline in the game.
Player *player_build() {
  Player *p = (Player *)calloc(sizeof(Player), 1);

  // general Object stuff
  p->type = OBJ_PLAYER;

  { // setup player phys data.
    Collider *colliders = (Collider *)calloc(sizeof(Collider), 1);
    make_colliders(1, colliders);
    colliders[0].type = CL_SPHERE;
    SphereColliderData *col_data =
        (SphereColliderData *)malloc(sizeof(SphereColliderData) * 1);
    col_data->radius = 1;
    colliders[0].data = col_data;

    float player_mass = 4.0;
    p->phys = make_physcomp(1, player_mass, 0.3, false, false, colliders, 1,
                            (vec3){0, 0, 0}, false);

    // sanity test, make sure that the array is setting and returning properly.
    assert(p->phys->mass == player_mass);
  }

  { // setup player rendering data.

    // parse then mesh the glb file, then render it in the normal drawing loop.
    p->render = make_rendercomp_from_glb(MODEL_PATH("wiggle.glb"));

    Model *player_model = (Model *)(p->render->data);
    p->animator = make_animator(player_model);

    // just for now, link the first root node and assume that's the one with
    // influence over the player's position.
    p->animation_root = &(player_model->nodes[player_model->roots[0]]);

    anim_play(p->animator, "wiggle", true);
  }

  // specific Player stuff.
  p->forward_speed = 8.0F;

  p->jump_power = 1000.0F;

  return p;
}

void player_init(void *p) {}

// Update player's mats and movement
static void update_player_mats_movement(Player *player) {}

static void player_handle_interactions(Player *player) { Position p; }

// The main function
static void player_handle_walking_state(Player *player) {
  player_handle_interactions(player);

  {
    mat4 offset_m;
    glm_mat4_identity(offset_m);
    glm_translate(offset_m, player->phys->lerp_position);

    vec2 v_move;
    get_movement_vec(v_move, i_state.act_held);

    if (fabs(v_move[0]) < 0.001F && fabs(v_move[1]) < 0.001F) {
      // no movement, don't bother
    } else {
      glm_vec2_scale(v_move, player->forward_speed, v_move);

      Basis b = get_basis(m_view);

      b.forward[1] = 0; // don't move vertically for now.
      b.right[1] = 0;

      { // then, generate the player's force push and apply it.
        vec3 force_direction = {0};

        glm_vec3_scale(b.right, v_move[0], b.right);
        glm_vec3_add(force_direction, b.right, force_direction);
        glm_vec3_scale(b.forward, v_move[1], b.forward);
        glm_vec3_add(force_direction, b.forward, force_direction);

        physics_apply_force((PhysComp *)player->phys, force_direction);
      }

      // then, take ANOTHER step, and use that to calculate the lookat matrix
      // for the proper model rotation. lazy!!!
      glm_vec3_copy(player->phys->position, player->ghost_step);

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
  Model *player_model = player->render->data;

  player->ghost_step[1] =
      player->phys->lerp_position[1]; // make the lookahead flat,
                                      // or else it looks weird.

  glm_mat4_identity(player_model->transform);
  glm_translate(player_model->transform, player->phys->lerp_position);
  glm_translate(player_model->transform, player->animation_root->translation);
  glm_lookat(player->phys->lerp_position, player->ghost_step, (vec3){0, 1, 0},
             player_model->transform);
  glm_mat4_inv(player_model->transform, player_model->transform);
  glm_rotate(player_model->transform, glm_rad(180), (vec3){0, 1, 0});

  { // handle jumping, unwrap the MQ and check for any floor collisions on the
    // previous physics tick.
    player->is_on_floor = false;

    WQueue *mailbox = &(player->phys->colliders[0].phys_events);
    while (mailbox->active_elements > 0) {
      PhysicsEvent *e = w_dequeue(mailbox);
      assert(
          e !=
          NULL); // shouldn't happen with the active elements condition above.
      if (e->sender_col_type == CL_FLOOR) {
        player->is_on_floor = true;
      }
    }

    if (player->is_on_floor) {
      if (i_state.act_just_pressed[ACT_JUMP]) {
        physics_apply_force((PhysComp *)player->phys,
                            (vec3){0, player->jump_power, 0});
      }
    }
  }

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

// maybe don't have a seperate destructor? is there a point to that?
void player_clean(void *p) {
  Player *player = (Player *)p;
  free(player);
}
