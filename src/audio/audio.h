#pragma once

// there's really not much in common here, other than the basic necessity of
// some lifetime functions. i won't lose sleep over it.

void a_init();
void a_update();
void a_clean();
void a_play_pcm(const char *filename); // play a wav file by path.
void a_stop_all();
