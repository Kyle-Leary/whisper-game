// define lighting helper functions for the graphics api that just manipulate
// the global structure.

#include "light.h"
#include "macros.h"

LightData g_light_data = {0};

DEFINE_CONTIG_ARRAY_SOURCE(PointLight, POINT_LIGHT_SLOTS,
                           { ERROR_NO_ARGS("ERROR: pointlight CA failed.\n"); })
DEFINE_CONTIG_ARRAY_SOURCE(DirectionalLight, DIRECTIONAL_LIGHT_SLOTS, {})
DEFINE_CONTIG_ARRAY_SOURCE(SpotLight, SPOT_LIGHT_SLOTS, {})
