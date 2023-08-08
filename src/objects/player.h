#ifndef PLAYER_H
#define PLAYER_H

#include "../object_lut.h"

#include "../cglm/types.h"
#include "../object.h"
#include "../physics.h"
#include "backends/graphics_api.h"
#include "defines.h"

// the camera is basically an object that controls and modifies the view matrix.
typedef struct Player {
  PHYS_OBJECT_FIELDS

  GraphicsRender *render;

  vec3 ghost_step; // used internally for calculating the "next" lookat position
                   // for the player model.

  float forward_speed;
} Player;

Player *player_build();
void player_destroy(Player *p);

void player_init(void *p);
void player_update(void *p);
void player_draw(void *p);
void player_clean(void *p);
void player_handle_collision(void *p, CollisionEvent *e);

#endif // !PLAYER_H
