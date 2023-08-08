#include "input_help.h"
#include "cglm/vec2.h"

#define THRESHOLD 0.001F

void get_movement_vec(vec2 v, u8 *input_substate) {
  v[0] = 0;
  v[1] = 0;

  // act generically over an input substate, like those found in
  // i_state.act_held and act_just_pressed.
  v[1] += input_substate[ACT_UP];
  v[1] -= input_substate[ACT_DOWN];
  v[0] += input_substate[ACT_RIGHT];
  v[0] -= input_substate[ACT_LEFT];

  // don't normalize zero.
  if (fabs(v[0]) > THRESHOLD && fabs(v[1]) > THRESHOLD)
    glm_vec2_normalize(v);
}
