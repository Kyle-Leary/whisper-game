#pragma once

#include "helper_math.h"

void gui_label(const char *text, AABB *aabb);
bool gui_button(const char *text, AABB *aabb);
void gui_draggable(const char *name, AABB *aabb);

void gui_init();
void gui_update();
void gui_draw();
void gui_clean();
