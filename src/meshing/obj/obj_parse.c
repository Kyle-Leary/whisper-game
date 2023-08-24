#include "obj_parse.h"
#include "backends/graphics_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  float x, y, z;
} Position;

typedef struct {
  float u, v;
} TextureCoord;

typedef struct {
  float x, y, z;
} Normal;

typedef struct { // indices into a greater normal, vert and tex vertex array.
  int v1, v2, v3;
  int t1, t2, t3;
  int n1, n2, n3;
} Face;

typedef struct {
  // parse directly into a nice array that fits into the non-interleaved
  // BasicVertexData format.
  Position *positions;
  TextureCoord *uvs;
  Normal *normals;
  Face *faces;
  int n_positions, n_uvs, n_normals, n_faces;
} OBJFile;

#define VERT_LIMIT 4096

// tris, not quads for this.
static void parseOBJ(const char *filepath,
                     OBJFile *obj_file) { // fill in the caller's allocated
                                          // OBJFile with data, which
  obj_file->n_positions = 0;
  obj_file->n_uvs = 0;
  obj_file->n_normals = 0;

  obj_file->n_faces = 0;

  Position positions[VERT_LIMIT];
  TextureCoord uvs[VERT_LIMIT];
  Normal normals[VERT_LIMIT];
  Face faces[VERT_LIMIT];

  // will be interpreted however way by the caller.
  FILE *file = fopen(filepath, "r");
  if (file == NULL) {
    fprintf(stderr, "Could not open the file: %s\n", filepath);
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    if (line[0] == 'v' && line[1] == ' ') {
      Position position;
      sscanf(line, "v %f %f %f", &position.x, &position.y, &position.z);

      memcpy(&positions[obj_file->n_positions], &position, sizeof(position));
      obj_file->n_positions++;
    } else if (line[0] == 'v' && line[1] == 't') {
      TextureCoord texCoord;
      sscanf(line, "vt %f %f", &texCoord.u, &texCoord.v);

      memcpy(&uvs[obj_file->n_uvs], &texCoord, sizeof(TextureCoord));
      obj_file->n_uvs++;
    } else if (line[0] == 'v' && line[1] == 'n') {
      Normal normal;
      sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);

      memcpy(&normals[obj_file->n_normals], &normal, sizeof(Normal));
      obj_file->n_normals++;
    } else if (line[0] == 'f') {
      Face face;
      sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &face.v1, &face.t1, &face.n1,
             &face.v2, &face.t2, &face.n2, &face.v3, &face.t3, &face.n3);

      memcpy(&faces[obj_file->n_faces], &face, sizeof(Face));
      obj_file->n_faces++;
    }
  }

  obj_file->positions =
      (Position *)malloc(sizeof(Position) * obj_file->n_positions);
  obj_file->uvs =
      (TextureCoord *)malloc(sizeof(TextureCoord) * obj_file->n_uvs);
  obj_file->normals = (Normal *)malloc(sizeof(Normal) * obj_file->n_normals);
  obj_file->faces = (Face *)malloc(sizeof(Face) * obj_file->n_faces);

  memcpy(obj_file->positions, positions,
         sizeof(Position) * obj_file->n_positions);
  memcpy(obj_file->uvs, uvs, sizeof(TextureCoord) * obj_file->n_uvs);
  memcpy(obj_file->normals, normals, sizeof(Normal) * obj_file->n_normals);
  memcpy(obj_file->faces, faces, sizeof(Face) * obj_file->n_faces);

  fclose(file);
}

GraphicsRender *render_from_obj(const char *filepath) {
  OBJFile f;
  parseOBJ(filepath, &f);

  unsigned int numVertices = f.n_positions;
  unsigned int numFaces = f.n_faces;

  // Each face consists of 3 vertices, so the number of indices is 3 times the
  // number of faces
  unsigned int numIndices = numFaces * 3;

  // Assuming each vertex consists of 8 floats (position x3, color x3, uv x2)
  float *vertices = (float *)malloc(numVertices * 8 * sizeof(float));
  unsigned int *indices =
      (unsigned int *)malloc(numIndices * sizeof(unsigned int));

  // Fill in indices
  for (int i = 0; i < numFaces; i++) {
    indices[i * 3 + 0] = f.faces[i].v1 - 1;
    indices[i * 3 + 1] = f.faces[i].v2 - 1;
    indices[i * 3 + 2] = f.faces[i].v3 - 1;
  }

  // Create GraphicsRender object
  GraphicsRender *gr = g_new_render(
      (VertexData *)&(BasicVertexData){RC_BASIC, numVertices,
                                       (float *)f.positions, (float *)f.normals,
                                       (float *)f.uvs},
      indices, numIndices);

  // Clean up temporary OBJFile data
  free(f.positions);
  free(f.uvs);
  free(f.normals);
  free(f.faces);
  free(vertices);
  free(indices);

  return gr;
}
