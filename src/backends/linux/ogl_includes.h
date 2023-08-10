#pragma once

// modern opengl includes specific to this backend.

#include "backends/graphics_api.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <GL/gl.h>

// also share state between the backend globally here.
extern GLFWwindow *window;

// global shader locations
extern int loc_model; // mvp locs
extern int loc_view_rot;
extern int loc_view_tf;
extern int loc_projection;

extern int loc_main_slot; // the texture slot we're rendering with.
extern int loc_u_time;    // general purpose time variable

// ui hud shader locations.
extern int loc_ui_model;
extern int loc_ui_projection;
extern int loc_ui_u_time;

extern int loc_ui_font_slot;
extern int loc_ui_text_color;

extern GLuint basic_program;
extern GLuint hud_program;

// only one Material can be bound at a time.
// generally, Materials are processed one mesh at a time.
extern Material *curr_mat;
