#pragma once

#include "helper_math.h"

// public API
void gui_widget(const char *name, AABB *aabb);
void gui_label(const char *name, const char *text, AABB *aabb);
bool gui_button(const char *name, const char *text, AABB *aabb);
void gui_draggable(const char *name, AABB *aabb);

void gui_push();
void gui_pop();

// lifecycle
void gui_init();
void gui_update();
void gui_draw();
void gui_clean();
