#pragma once

typedef struct GLFWwindow GLFWwindow;
extern GLFWwindow *window;

void window_init();
int window_should_close();
void window_begin_draw();
void window_update();
void window_end_draw();
void window_clean();