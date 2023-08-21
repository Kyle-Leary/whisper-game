#pragma once

// scale down or up the base FPS that serves as the game's heartbeat.
#define BASE_FPS (60.0)
#define BASE_FRAME_TIME (1.0 / BASE_FPS)

extern float fps;
extern float frame_time;

void timescale_init();
void timescale_update();

void timescale_scale_by(float scale);
