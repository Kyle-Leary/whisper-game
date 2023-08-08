#pragma once

#include "backends/graphics_api.h"
#include "parsers/area.h"

GraphicsRender *make_flat_dungeon_render(AreaFile *area, vec3 position);
