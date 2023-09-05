#pragma once

#include <sys/types.h>

#include "helper_math.h"
#include <stdbool.h>

void gui_widget(const char *name);
void gui_label(const char *name, const char *text);
bool gui_button(const char *name, const char *text);
void gui_draggable(const char *name);

void gui_widget_types_init();
