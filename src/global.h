#pragma once
// container for dumb externs of global state.

#include "cglm/cglm.h"
#include "cglm/types.h"
#include "main.h"
#include "meshing/font.h"

#include <sys/types.h>

extern int win_w;
extern int win_h;

// these are global and cross-backend. it's just a matrix, we can handle the
// difference between actually setting the matrix in the graphics_api.h file.
extern mat4 m_model;
extern mat4 m_view;
extern mat4 m_projection;

extern mat4 m_ui_model;
extern mat4 m_ui_projection;

extern float delta_time;
extern float u_time; // a sum of all the time passed, usually used for shader
                     // stuff like sin(u_time) * mag for waves.

extern Font *simple_font;

// basic movement.
extern float rot_dx;
extern float forward_dx;
