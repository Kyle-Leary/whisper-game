#pragma once

// this is a catch-all "backend-specific code" module.
// anything that doesn't really fit in another api is just handled here.
// for example, making a window is really linux-specific, and some of the other
// backends don't even need that, so we handle that on linux through
// lifecycle_api.h instead of making a new window_api.h just for one backend.

// can all return error codes, remember this is basically the main function,
// just generalized to a set of lifecycle functions.
int l_init();
int l_should_close(); // should the main loop stop?
int l_begin_draw();
int l_update();
int l_end_draw();
int l_clean();
