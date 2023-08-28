#pragma once

#define LINE_BUF_SZ 256

void toggle_console();

void console_handle_input(int key, int scancode, int action, int mods);

void console_init();
void console_update();
void console_draw();
void console_clean();

// these are really only intended for the command runners.
void console_newline();
void console_print(char *text, int len);
void console_println(char *line_text, int len);
void console_printf(const char *format, ...);
