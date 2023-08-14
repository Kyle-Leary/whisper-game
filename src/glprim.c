#include "glprim.h"
#include "backends/graphics_api.h"
#include "defines.h"
#include "helper_math.h"
#include "hud.h"
#include "main.h"
#include "printers.h"
#include "util.h"
#include <string.h>

static float cubePositions[] = {-0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
                                0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,
                                -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f,
                                0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f};

static float cubeNormals[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                              1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                              1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};

static float cubeUVs[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                          1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};

static unsigned int cubeIndices[] = {
    // Front face
    0, 1, 2, // Triangle 1
    2, 3, 0, // Triangle 2
    // Right face
    1, 5, 6, // Triangle 1
    6, 2, 1, // Triangle 2
    // Back face
    7, 6, 5, // Triangle 1
    5, 4, 7, // Triangle 2
    // Left face
    4, 0, 3, // Triangle 1
    3, 7, 4, // Triangle 2
    // Bottom face
    4, 5, 1, // Triangle 1
    1, 0, 4, // Triangle 2
    // Top face
    3, 2, 6, // Triangle 1
    6, 7, 3  // Triangle 2
};

static unsigned int cubeNumVertices = 8; // Number of vertices in the cube
static unsigned int cubeNumIndices = 36; // Number of indices in the cube

// just returns the GraphicsRender, you have to set it yourself.
GraphicsRender *glprim_cube(vec3 position) {
  // more typepunning bullshit hahaha what the fuck
  GraphicsRender *gr = g_new_render(
      (VertexData *)&(BasicVertexData){RC_BASIC, cubeNumVertices, cubePositions,
                                       cubeNormals, cubeUVs},
      cubeIndices, cubeNumIndices);

  glm_translate(gr->model, position);
  return gr;
}

// all at 1.0, good for skyboxes and projection of cubemaps.
static float skyboxCubePositions[] = {-1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
                                      1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,
                                      -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                                      1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f};

static float skyboxCubeNormals[] = {
    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};

static float skyboxCubeUVs[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                                1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};

static unsigned int skyboxCubeIndices[] = {
    // Front face
    0, 1, 2, // Triangle 1
    2, 3, 0, // Triangle 2
    // Right face
    1, 5, 6, // Triangle 1
    6, 2, 1, // Triangle 2
    // Back face
    7, 6, 5, // Triangle 1
    5, 4, 7, // Triangle 2
    // Left face
    4, 0, 3, // Triangle 1
    3, 7, 4, // Triangle 2
    // Bottom face
    4, 5, 1, // Triangle 1
    1, 0, 4, // Triangle 2
    // Top face
    3, 2, 6, // Triangle 1
    6, 7, 3  // Triangle 2
};

static unsigned int skyboxCubeNumVertices = 8;
static unsigned int skyboxCubeNumIndices = 36;

GraphicsRender *glprim_skybox_cube() {
  GraphicsRender *gr = g_new_render(
      (VertexData *)&(BasicVertexData){RC_BASIC, skyboxCubeNumVertices,
                                       skyboxCubePositions, skyboxCubeNormals,
                                       skyboxCubeUVs},
      skyboxCubeIndices, skyboxCubeNumIndices);
  return gr;
}

static float planePositions[] = {-0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f,
                                 0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f};

static float planeNormals[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};

static float planeUVs[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

// Cube index data for non-indexed rendering (36 indices)
static unsigned int planeIndices[] = {
    // Front face
    0, 1, 2, // Triangle 1
    2, 3, 0, // Triangle 2
};

static unsigned int planeNumVertices = 4; // Number of vertices in the plane
static unsigned int planeNumIndices = 6;  // Number of indices in the plane

// plane facing up, something that can easily billboard the character and
// display a sprite, for example.
GraphicsRender *glprim_upright_plane(vec3 position) {
  GraphicsRender *gr = g_new_render(
      (VertexData *)&(BasicVertexData){RC_BASIC, planeNumVertices,
                                       planePositions, planeNormals, planeUVs},
      cubeIndices, cubeNumIndices);
  glm_translate(gr->model, position); // translates it for the caller.
  return gr;
}

static float floor_plane_positions[] = {-1.0f, 0.0f, -1.0f, 1.0f,  0.0f, -1.0f,
                                        1.0f,  0.0f, 1.0f,  -1.0f, 0.0f, 1.0f};

static float floor_plane_normals[] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                      0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

// UVs stay the same as your previous example
static float floor_plane_uvs[] = {0.0f, 0.0f, 1.0f, 0.0f,
                                  1.0f, 1.0f, 0.0f, 1.0f};

// Indices stay the same as well
static unsigned int floor_plane_indices[] = {
    // Front face
    0, 1, 2, // Triangle 1
    2, 3, 0, // Triangle 2
};

GraphicsRender *glprim_floor_plane(vec3 position) {
  GraphicsRender *gr = g_new_render(
      (VertexData *)&(BasicVertexData){RC_BASIC, planeNumVertices,
                                       floor_plane_positions,
                                       floor_plane_normals, floor_plane_uvs},
      floor_plane_indices, planeNumIndices);
  glm_translate(gr->model, position); // translates it for the caller.
  return gr;
}

// Function to create a renderable rectangle from an AABB
GraphicsRender *glprim_ui_rect(AABB aabb) {
  // Define the 4 corner points of the rectangle
  vec2 p1 = {aabb.xy[0], aabb.xy[1]};              // Bottom-left corner
  vec2 p2 = {aabb.xy[0] + aabb.wh[0], aabb.xy[1]}; // Bottom-right corner
  vec2 p3 = {aabb.xy[0] + aabb.wh[0],
             aabb.xy[1] + aabb.wh[1]};             // Top-right corner
  vec2 p4 = {aabb.xy[0], aabb.xy[1] + aabb.wh[1]}; // Top-left corner

  // Define the UV coordinates for the rectangle (assuming it covers the entire
  // texture)
  vec2 uv1 = {0.0f, 0.0f};
  vec2 uv2 = {1.0f, 0.0f};
  vec2 uv3 = {1.0f, 1.0f};
  vec2 uv4 = {0.0f, 1.0f};

  //// just stack-allocate the array data, it'll be written into the ebo and vbo
  //// then go out of scope when this function returns.
  // Create the vertex array for the rectangle
  float rectPositions[4 * 2] = {
      p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1],
  };

  float rectUVs[4 * 2] = {0, 0, 1, 0, 1, 1, 0, 1};

  // Define the vertex indices for the rectangle
  unsigned int indices[3 * 2] = {
      0, 1,
      2, // First triangle
      2, 3,
      0, // Second triangle
  };

  // Create the render object and set its data
  GraphicsRender *gr = g_new_render(
      (VertexData *)&(HUDVertexData){RC_HUD, 4, rectPositions, rectUVs},
      indices, 6);

  return gr;
}

GraphicsRender *glprim_sphere(vec3 position, float radius,
                              unsigned int segments) {
  unsigned int numVertices = (segments + 1) * (segments + 1);
  unsigned int numIndices = segments * segments * 6;

  float *spherePositions = malloc(sizeof(float) * 3 * numVertices);
  float *sphereNormals = malloc(sizeof(float) * 3 * numVertices);
  float *sphereUVs = malloc(sizeof(float) * 2 * numVertices);
  unsigned int *sphereIndices = malloc(sizeof(unsigned int) * numIndices);

  float segmentSize = M_PI * 2.0f / segments;
  unsigned int vertexIndex = 0, normalIndex = 0, uvIndex = 0, indexIndex = 0;

  for (unsigned int y = 0; y <= segments; y++) {
    for (unsigned int x = 0; x <= segments; x++) {
      float xSegment = (float)x / (float)segments;
      float ySegment = (float)y / (float)segments;
      float xPos = cos(xSegment * M_PI * 2.0f) * sin(ySegment * M_PI) * radius;
      float yPos = cos(ySegment * M_PI) * radius;
      float zPos = sin(xSegment * M_PI * 2.0f) * sin(ySegment * M_PI) * radius;

      // Position
      spherePositions[vertexIndex++] = xPos;
      spherePositions[vertexIndex++] = yPos;
      spherePositions[vertexIndex++] = zPos;

      // Normal (same as position for unit sphere, may need to normalize)
      sphereNormals[normalIndex++] = xPos / radius;
      sphereNormals[normalIndex++] = yPos / radius;
      sphereNormals[normalIndex++] = zPos / radius;

      // UV
      sphereUVs[uvIndex++] = xSegment;
      sphereUVs[uvIndex++] = ySegment;

      // Indices
      if (x < segments && y < segments) {
        unsigned int i0 = y * (segments + 1) + x;
        unsigned int i1 = y * (segments + 1) + x + 1;
        unsigned int i2 = (y + 1) * (segments + 1) + x;
        unsigned int i3 = (y + 1) * (segments + 1) + x + 1;
        sphereIndices[indexIndex++] = i0;
        sphereIndices[indexIndex++] = i2;
        sphereIndices[indexIndex++] = i1;
        sphereIndices[indexIndex++] = i1;
        sphereIndices[indexIndex++] = i2;
        sphereIndices[indexIndex++] = i3;
      }
    }
  }

  GraphicsRender *gr = g_new_render(
      (VertexData *)&(BasicVertexData){RC_BASIC, numVertices, spherePositions,
                                       sphereNormals, sphereUVs},
      sphereIndices, numIndices);
  glm_translate(gr->model, position); // translates it for the caller.

  // Free temporary buffers
  free(spherePositions);
  free(sphereNormals);
  free(sphereUVs);
  free(sphereIndices);

  return gr;
}
