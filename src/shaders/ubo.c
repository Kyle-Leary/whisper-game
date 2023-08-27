#include "ubo.h"

#include "backends/ogl_includes.h"
#include "global.h"
#include <sys/types.h>

// go by the index specified in the enum variant.
static uint ubo_ids[BLOCK_COUNT];

// setup the ids and data ranges.
void ubo_init() {
  glGenBuffers(4, ubo_ids);

  glBindBuffer(GL_UNIFORM_BUFFER, ubo_ids[LIGHT_BLOCK]);
  // we're going to change the ubo frequently with subdata calls, so make this
  // dynamic.
  glBufferData(GL_UNIFORM_BUFFER, sizeof(g_light_data), NULL, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_UNIFORM_BUFFER, LIGHT_BLOCK, ubo_ids[LIGHT_BLOCK], 0,
                    sizeof(g_light_data));

  // space for two matrices. this doesn't require padding like the light data.
  glBindBuffer(GL_UNIFORM_BUFFER, ubo_ids[MATRIX_BLOCK]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 32, NULL, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_UNIFORM_BUFFER, MATRIX_BLOCK, ubo_ids[MATRIX_BLOCK], 0,
                    sizeof(float) * 32);

  glBindBuffer(GL_UNIFORM_BUFFER, ubo_ids[BONE_BLOCK]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(BoneData), NULL, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_UNIFORM_BUFFER, BONE_BLOCK, ubo_ids[BONE_BLOCK], 0,
                    sizeof(BoneData));

  glBindBuffer(GL_UNIFORM_BUFFER, ubo_ids[MATERIAL_BLOCK]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialData), NULL, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_UNIFORM_BUFFER, MATERIAL_BLOCK, ubo_ids[MATERIAL_BLOCK],
                    0, sizeof(MaterialData));
}

void ubo_update() { // update UBOs.
  // these will just change per frame usually, so update them in a loop
  // instead of making a whole new graphics api function just to change them.
  { // light
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_ids[LIGHT_BLOCK]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(g_light_data), &g_light_data);
  }

  { // matrix
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_ids[MATRIX_BLOCK]);
    // view, then the projection.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, m_view);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16,
                    m_projection);
  }

  // we're not updating bone ubo in a loop, just update that before we draw a
  // model, calling the specified graphics api function.
}
