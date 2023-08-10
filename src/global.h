#pragma once
// container for dumb externs of global state.

#include "backends/graphics_api.h"
#include "cglm/cglm.h"
#include "cglm/types.h"
#include "main.h"
#include "meshing/font.h"
#include "objects/player.h"

#include <sys/types.h>

// these are global and cross-backend. it's just a matrix, we can handle the
// difference between actually setting the matrix in the graphics_api.h file.
extern mat4 m_model;
extern mat4 m_view;
extern mat4 m_projection;

extern mat4 m_ui_model;
extern mat4 m_ui_projection;

extern Font *ui_font;

extern float delta_time;

// basic movement.
extern float rot_dx;
extern float forward_dx;
