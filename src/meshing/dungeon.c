#include "dungeon.h"
#include "backends/graphics_api.h"
#include "cglm/affine-pre.h"
#include "cglm/types.h"
#include "core/tile.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

#define MAX_AREA_X 100
#define MAX_AREA_Y 100

// 4 verts with 8 floats per tile
#define MAX_VERT (MAX_AREA_X * MAX_AREA_Y * 4 * 8 * sizeof(float))
// 6 indices per tile to render 2 tris for the rect
#define MAX_IDX (MAX_AREA_X * MAX_AREA_Y * 6)

static float positions[MAX_VERT * 3];
static float normals[MAX_VERT * 3];
static float uvs[MAX_VERT * 2];
static unsigned int indices[MAX_IDX];

GraphicsRender *make_dungeon_render(AreaFile *area, vec3 position) {
  if (area == NULL || area->tiledata == NULL) {
    fprintf(stderr, "Invalid AreaFile or tiledata.\n");
    return NULL;
  }

  unsigned int vtx_i = 0, idx_i = 0;
  unsigned int n_verts = 0;
  int w = area->x_size, h = area->y_size;

  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      TileData t = area->tiledata[j][i];

      // Define the tile's vertices, normals, and UVs
      static float t_positions[8 * 4 * 3];
      static float t_normals[8 * 4 * 3];
      static float t_uvs[8 * 4 * 2];
      static unsigned int t_idx[3 * 2 * 4];

      unsigned int vtx_pos_i = 0;
      unsigned int vtx_normal_i = 0;
      unsigned int vtx_uv_i = 0;
      unsigned int idx_i = 0;

      if (t.type == TT_FLOOR) {
        // ceiling and floor planes rendered when there's an open "floor" tile.
        float t_positions[3 * 4 * 2] = {
            // bottom face
            -0.5F + i,
            0,
            -0.5F + j,
            0.5F + i,
            0,
            -0.5F + j,
            0.5F + i,
            0,
            0.5F + j,
            -0.5F + i,
            0,
            0.5F + j,
            // top face
            -0.5F + i,
            4,
            -0.5F + j,
            0.5F + i,
            4,
            -0.5F + j,
            0.5F + i,
            4,
            0.5F + j,
            -0.5F + i,
            4,
            0.5F + j,
        };

        float t_normals[3 * 4 * 2] = {
            // bottom face
            1,
            0,
            1,
            0.5,
            0,
            -1,
            1,
            0,
            1,
            0,
            1,
            -1,
            // top face
            1,
            0,
            1,
            0.5,
            0,
            -1,
            1,
            0,
            1,
            0,
            1,
            -1,
        };

        float t_uvs[2 * 4 * 2] = {
            // bottom face
            1,
            0,
            0.5,
            0,
            1,
            0,
            0,
            1,
            // top face
            1,
            0,
            0.5,
            0,
            1,
            0,
            0,
            1,
        };

        // only render the top and bottom.
        unsigned int t_idx[3 * 2 * 2] = {
            // bottom face
            0 + n_verts,
            1 + n_verts,
            2 + n_verts,
            2 + n_verts,
            3 + n_verts,
            0 + n_verts,

            // top face
            4 + n_verts,
            5 + n_verts,
            6 + n_verts,
            6 + n_verts,
            7 + n_verts,
            4 + n_verts,
        };

        n_verts += 8;

        memcpy(&position[vtx_pos_i], t_positions, 8 * 3 * sizeof(float));
        vtx_pos_i += 8 * 3;

        memcpy(&normals[vtx_normal_i], t_normals, 8 * 3 * sizeof(float));
        vtx_normal_i += 8 * 3;

        memcpy(&uvs[vtx_uv_i], t_uvs, 8 * 2 * sizeof(float));
        vtx_uv_i += 8 * 2;

        memcpy(&indices[idx_i], t_idx, 3 * 2 * 2 * sizeof(unsigned int));
        idx_i += 3 * 2 * 2;

      } else if (t.type == TT_WALL) {
        //   // all of the vert data for this one tile, to be memcpy'd later.
        //   float t_verts[8 * 4 * 2] = {
        //       //  bottom face
        //       -0.5F + i, 0, -0.5F + j, 1, 0, 0, 1, -1,   // red
        //       0.5F + i, 0, -0.5F + j, 1, 0.5, 0, -1, -1, // orange
        //       0.5F + i, 0, 0.5F + j, 0, 1, 0, 1, -1,     // green
        //       -0.5F + i, 0, 0.5F + j, 0, 0, 1, -1, 1,    // blue
        //
        //       // top face
        //       -0.5F + i, 4, -0.5F + j, 1, 0, 0, 1, -1,   // red
        //       0.5F + i, 4, -0.5F + j, 1, 0.5, 0, -1, -1, // orange
        //       0.5F + i, 4, 0.5F + j, 0, 1, 0, 1, -1,     // green
        //       -0.5F + i, 4, 0.5F + j, 0, 0, 1, -1, 1,    // blue
        //   };
        //
        //   // four faces, don't need the top and bottom for this.
        //   unsigned int t_idx[3 * 2 * 4] = {
        //       // // bottom face
        //       // 0 + n_verts,
        //       // 1 + n_verts,
        //       // 2 + n_verts,
        //       // 2 + n_verts,
        //       // 3 + n_verts,
        //       // 0 + n_verts,
        //       //
        //       // // top face
        //       // 4 + n_verts,
        //       // 5 + n_verts,
        //       // 6 + n_verts,
        //       // 6 + n_verts,
        //       // 7 + n_verts,
        //       // 4 + n_verts,
        //
        //       // front face
        //       0 + n_verts,
        //       1 + n_verts,
        //       5 + n_verts,
        //       5 + n_verts,
        //       4 + n_verts,
        //       0 + n_verts,
        //
        //       // back face
        //       2 + n_verts,
        //       3 + n_verts,
        //       7 + n_verts,
        //       7 + n_verts,
        //       6 + n_verts,
        //       2 + n_verts,
        //
        //       // left face
        //       0 + n_verts,
        //       3 + n_verts,
        //       7 + n_verts,
        //       7 + n_verts,
        //       4 + n_verts,
        //       0 + n_verts,
        //
        //       // right face
        //       1 + n_verts,
        //       2 + n_verts,
        //       6 + n_verts,
        //       6 + n_verts,
        //       5 + n_verts,
        //       1 + n_verts,
        //   };
        // }
        //
        // // Copy the data into the static arrays
        // memcpy(&positions[vtx_i * 3], t_positions, 8 * 4 * 3 *
        // sizeof(float)); memcpy(&normals[vtx_i * 3], t_normals, 8 * 4 * 3 *
        // sizeof(float)); memcpy(&uvs[vtx_i * 2], t_uvs, 8 * 4 * 2 *
        // sizeof(float)); memcpy(&indices[idx_i], t_idx, 3 * 2 * 4 *
        // sizeof(unsigned int));
        //
        // vtx_i += 8 * 4;
        // idx_i += 3 * 2 * 4;
        // n_verts += 8;
      }
    }
  }

  // Assuming you have a function to handle these separate arrays
  GraphicsRender *gr =
      g_new_render((VertexData *)&(BasicVertexData){RC_BASIC, n_verts,
                                                    positions, normals, uvs},
                   indices, idx_i);
  glm_translate(gr->model, position);

  return gr;
}
