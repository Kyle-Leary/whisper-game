// organize all of the globals here, rather than defining in random spots.
#include "global.h"
#include "cglm/types.h"

int win_w = 1700;
int win_h = 820;

mat4 m_model = {0};
mat4 m_view = {0};
mat4 m_projection = {0};

mat4 m_ui_model = {0};
mat4 m_ui_projection = {0};

Font *ui_font = NULL;

float delta_time = 0.016F;
float u_time = 0;

float rot_dx = 0;
float forward_dx = 0;
