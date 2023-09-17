#pragma once

#include "cglm/types.h"
#include "shaders/shader_instances.h"
#include "whisper/contig_array.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/* define light structures that will be passed directly into a PBR-type shader.
 */

#define POINT_LIGHT_SLOTS 5
#define SPOT_LIGHT_SLOTS 5
#define DIRECTIONAL_LIGHT_SLOTS 5

// an index into a light array, operating as a "light slot", since the lights
// are simply stored as elements in a global array.
typedef uint LightSlot;

typedef struct SpotLight {
  vec3 position;
  char pad1[4];
} SpotLight;

typedef struct PointLight {
  vec3 position;
  char pad1[4];
  vec4 color;
  // vec4 is already padded
  float intensity;
  float falloff;
  char pad2[8];
} PointLight;

typedef struct DirectionalLight {
  vec3 direction;
  char pad1[4];
  vec4 color;
  float intensity;
  char pad2[12];
} DirectionalLight;

// this is the odd one out, only one instance. a kind of global illumination in
// the scene.
typedef struct AmbientLight {
  vec4 color;
  float intensity;
  char pad1[12];
} AmbientLight;

// 12 bytes of padding at the end of the contig array, the W_CA_PointLight
// structure will be stuck directly in the light data.
DEFINE_CONTIG_ARRAY_TYPES(PointLight, POINT_LIGHT_SLOTS, , , char padding[12];)
DEFINE_CONTIG_ARRAY_TYPES(DirectionalLight, DIRECTIONAL_LIGHT_SLOTS, , ,
                          char padding[12];)
DEFINE_CONTIG_ARRAY_TYPES(SpotLight, SPOT_LIGHT_SLOTS, , , char padding[12];)

// the global structure that holds all the light data in the scene. one of these
// should be active at a time, and it should generally be managed through helper
// methods.
typedef struct LightData {
  // shaders can only store and pass around fixed-width data, so we need this
  // all structured at compile-time. that's the point of "slots" rather than
  // variable-sized data in a shader, it's just how cpu gpu communication works.

  // these MUST BE IN THE RIGHT ORDER. BE VERY CAREFUL WITH THE LAYOUT OF THIS
  // STRUCTURE, IT IS EXTREMELY DELIBERATE.
  W_CA_SpotLight spot_light_ca;
  W_CA_PointLight point_light_ca;
  W_CA_DirectionalLight directional_light_ca;

  AmbientLight ambient_light;
} LightData;

// in the shader, the structure is exactly the same so we don't need to worry
// about drilling past the ContiguousArray stuff in the glsl code/structs.

// the contigarray functions manipulate this global structure directly.
extern LightData g_light_data;

// define functions for the manip of contiguous light arrays that should be sent
// to the shader.
DEFINE_CONTIG_ARRAY_HEADERS(PointLight, POINT_LIGHT_SLOTS)
DEFINE_CONTIG_ARRAY_HEADERS(DirectionalLight, DIRECTIONAL_LIGHT_SLOTS)
DEFINE_CONTIG_ARRAY_HEADERS(SpotLight, SPOT_LIGHT_SLOTS)
