#include "flat_dungeon.h"
#include "backends/graphics_api.h"
#include "cglm/affine-pre.h"
#include "cglm/types.h"
#include "core/tile.h"
#include "main.h"
#include <string.h>

#include <stdio.h>

#define MAX_AREA_X 100
#define MAX_AREA_Y 100

// 4 verts with 8 floats per tile
#define MAX_VERT (MAX_AREA_X * MAX_AREA_Y * 4 * 8 * sizeof(float))
// 6 indices per tile to render 2 tris for the rect
#define MAX_IDX (MAX_AREA_X * MAX_AREA_Y * 6)

GraphicsRender *make_flat_dungeon_render(AreaFile *area, vec3 position) {
  if (area == NULL) {
    fprintf(stderr, "AreaFile passed to make_flat_dungeon_render() is null.\n");
    return NULL;
  }

  if (area->tiledata == NULL) {
    fprintf(
        stderr,
        "AreaFile->tiledata passed to make_flat_dungeon_render() is null.\n");
    return NULL;
  }

  unsigned int n_verts = 0;
  int w = area->x_size;
  int h = area->y_size;

  static float positions[MAX_VERT / 3];
  static float normals[MAX_VERT / 3];
  static float uvs[MAX_VERT / 4];
  static unsigned int indices[MAX_IDX];

  unsigned int vtx_pos_i = 0;
  unsigned int vtx_normal_i = 0;
  unsigned int vtx_uv_i = 0;
  unsigned int idx_i = 0;

  int i, j;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      TileData t = area->tiledata[j][i];
      switch (t.type) {
      case TT_WALL:
        break;
      default:
        continue;
        break;
      }

      float t_positions[4 * 3] = {
          -0.5F + i, 0, -0.5F + j, 0.5F + i,  0, -0.5F + j,
          0.5F + i,  0, 0.5F + j,  -0.5F + i, 0, 0.5F + j,
      };

      float t_normals[4 * 3] = {
          0, -1, -1, 0, 1, -1, 0, 1, 1, 0, -1, 1,
      };

      float t_uvs[4 * 2] = {
          1, 0, 0.5, 0, 1, 0, 0, 1,
      };

      unsigned int t_idx[3 * 2] = {0 + n_verts, 1 + n_verts, 2 + n_verts,
                                   2 + n_verts, 3 + n_verts, 0 + n_verts};

      n_verts += 4;

      memcpy(&positions[vtx_pos_i], t_positions, 3 * 4 * sizeof(float));
      vtx_pos_i += 3 * 4;

      memcpy(&normals[vtx_normal_i], t_normals, 3 * 4 * sizeof(float));
      vtx_normal_i += 3 * 4;

      memcpy(&uvs[vtx_uv_i], t_uvs, 2 * 4 * sizeof(float));
      vtx_uv_i += 2 * 4;

      memcpy(&indices[idx_i], t_idx, 3 * 2 * sizeof(unsigned int));
      idx_i += 3 * 2;
    }
  }

  // You can print the arrays or handle them as per your requirements

  // You need to modify the g_new_render function to accept the separate arrays
  GraphicsRender *gr =
      g_new_render((VertexData *)&(BasicVertexData){RC_BASIC, n_verts,
                                                    positions, normals, uvs},
                   indices, idx_i);
  glm_translate(gr->model, position);

  return gr;
}
