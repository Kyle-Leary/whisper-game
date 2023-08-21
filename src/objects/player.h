#ifndef PLAYER_H
#define PLAYER_H

#include "../object_lut.h"

#include "../object.h"
#include "../physics.h"
#include "animation/animator.h"
#include "backends/graphics_api.h"
#include "cglm/types.h"
#include "defines.h"
#include "render.h"
#include <stdbool.h>

// the camera is basically an object that controls and modifies the view matrix.
typedef struct Player {
  OBJECT_FIELDS

  PhysComp *phys;
  RenderComp *render;
  Animator *animator;

  Node *animation_root; // a direct link to the Node in the Model hierarchy that
                        // has direct influence over the player's trs vectors.

  vec3 ghost_step; // used internally for calculating the "next" lookat position
                   // for the player model.

  bool is_on_floor;

  float forward_speed;
  float jump_power;
} Player;

Player *player_build();
void player_destroy(Player *p);

void player_init(void *p);
void player_update(void *p);
void player_clean(void *p);

#endif // !PLAYER_H
