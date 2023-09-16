#pragma once

#include "defines.h"
typedef struct GLFWwindow GLFWwindow;
extern GLFWwindow *window;

// in bytes.
int window_get_frame_buffer_size();
// pass in a buffer w/ appropriate size from window_get_frame_size().
void window_get_frame_buffer(byte *buf);

void window_init();
void window_force_close();
int window_should_close();
void window_begin_draw();
void window_update();
void window_end_draw();
void window_clean();
