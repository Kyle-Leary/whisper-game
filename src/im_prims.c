#include "im_prims.h"

#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "immediate.h"
#include "math/mat.h"
#include "mathdef.h"

#define VEL_COLOR                                                              \
  (vec4) { 1, 0, 0, 1 }

#define ACCEL_COLOR                                                            \
  (vec4) { 0, 1, 0, 1 }

#define POINT_COLOR                                                            \
  (vec4) { 1, 1, 1, 1 }

void im_velocity(RigidBody *rb) {
  vec3 line_positions[2];
  glm_vec3_copy(rb->position, line_positions[0]);
  glm_vec3_copy(rb->velocity, line_positions[1]);
  glm_vec3_add(line_positions[1], line_positions[0], line_positions[1]);
  im_draw((float *)line_positions, 2, VEL_COLOR, IM_LINE_STRIP);
}

void im_acceleration(RigidBody *rb) {
  vec3 line_positions[2];
  glm_vec3_copy(rb->position, line_positions[0]);
  glm_vec3_copy(rb->acceleration, line_positions[1]);
  glm_vec3_add(line_positions[1], line_positions[0], line_positions[1]);
  im_draw((float *)line_positions, 2, ACCEL_COLOR, IM_LINE_STRIP);
}

void im_vector(vec3 origin, vec3 endpoint, vec4 color) {
  // Create an array to store the positions of the line's endpoints
  vec3 line_positions[2];

  // Copy the given vectors into the positions array
  glm_vec3_copy(origin, line_positions[0]);
  glm_vec3_copy(endpoint, line_positions[1]);

  // Draw the line using your existing im_draw function
  im_draw((float *)line_positions, 2, color, IM_LINE_STRIP);
}

void im_cube(vec3 center, float side_length) {
  // Half-lengths for convenience
  float half_side = side_length / 2.0f;

  // Define 8 corners of the cube
  vec3 corners[8];
  for (int i = 0; i < 8; ++i) {
    corners[i][0] = center[0] + ((i & 1) ? half_side : -half_side);
    corners[i][1] = center[1] + ((i & 2) ? half_side : -half_side);
    corners[i][2] = center[2] + ((i & 4) ? half_side : -half_side);
  }

  // Define vertices for 12 triangles (two per face)
  vec3 vertices[36];
  int indices[36] = {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 0, 1, 4, 4, 1, 5,
                     2, 3, 6, 6, 3, 7, 0, 2, 4, 4, 2, 6, 1, 3, 5, 5, 3, 7};

  for (int i = 0; i < 36; ++i) {
    glm_vec3_copy(corners[indices[i]], vertices[i]);
  }

  // Draw the cube using IM_TRIANGLES
  im_draw((float *)vertices, 36, (vec4){1, 1, 1, 1}, IM_TRIANGLES);
}

void im_point(vec3 point) { im_cube(point, 0.04); }

void im_transform(mat4 m) {
  vec3 origin, x_axis, y_axis, z_axis;

  // Extract origin (translation)
  origin[0] = m[3][0];
  origin[1] = m[3][1];
  origin[2] = m[3][2];

  // Extract axes (rotation and scale)
  x_axis[0] = m[0][0];
  x_axis[1] = m[0][1];
  x_axis[2] = m[0][2];

  y_axis[0] = m[1][0];
  y_axis[1] = m[1][1];
  y_axis[2] = m[1][2];

  z_axis[0] = m[2][0];
  z_axis[1] = m[2][1];
  z_axis[2] = m[2][2];

  // Scale the axis lines to make them easier to see
  glm_vec3_scale(x_axis, 0.2f, x_axis);
  glm_vec3_scale(y_axis, 0.2f, y_axis);
  glm_vec3_scale(z_axis, 0.2f, z_axis);

  // Add origin offset
  glm_vec3_add(x_axis, origin, x_axis);
  glm_vec3_add(y_axis, origin, y_axis);
  glm_vec3_add(z_axis, origin, z_axis);

  // Draw lines from origin to each axis tip
  vec3 lines[6];
  glm_vec3_copy(origin, lines[0]);
  glm_vec3_copy(x_axis, lines[1]);
  glm_vec3_copy(origin, lines[2]);
  glm_vec3_copy(y_axis, lines[3]);
  glm_vec3_copy(origin, lines[4]);
  glm_vec3_copy(z_axis, lines[5]);

  // Render lines
  im_draw((float *)lines, 2, (vec4){1, 0, 0, 1}, IM_LINES); // X-axis in red
  im_draw((float *)lines + 6, 2, (vec4){0, 1, 0, 1},
          IM_LINES); // Y-axis in green
  im_draw((float *)lines + 12, 2, (vec4){0, 0, 1, 1},
          IM_LINES); // Z-axis in blue
}

void im_identity_grid(int size, float saturation) {
  for (int y = -size; y <= size; ++y) {
    for (int z = -size; z <= size; ++z) {
      // draw all the x basis lines
      im_vector((vec3){-size, y, z}, (vec3){size, y, z},
                (vec4){saturation, 0, 0, 1});
    }
  }

  for (int x = -size; x <= size; ++x) {
    for (int z = -size; z <= size; ++z) {
      im_vector((vec3){x, -size, z}, (vec3){x, size, z},
                (vec4){0, saturation, 0, 1});
    }
  }

  for (int y = -size; y <= size; ++y) {
    for (int x = -size; x <= size; ++x) {
      im_vector((vec3){x, y, -size}, (vec3){x, y, size},
                (vec4){0, 0, saturation, 1});
    }
  }
}

void im_grid(mat4 m, int size, float step) {
  vec3 point, transformed_point;
  vec3 origin, x_axis, y_axis, z_axis;

  // Extract origin (translation)
  origin[0] = m[3][0];
  origin[1] = m[3][1];
  origin[2] = m[3][2];

  // Extract axes (rotation and scale)
  x_axis[0] = m[0][0];
  x_axis[1] = m[0][1];
  x_axis[2] = m[0][2];

  y_axis[0] = m[1][0];
  y_axis[1] = m[1][1];
  y_axis[2] = m[1][2];

  z_axis[0] = m[2][0];
  z_axis[1] = m[2][1];
  z_axis[2] = m[2][2];

  {
    // draw all the x basis lines
    vec3 x_scaled;
    vec3 x_origin, x_endpoint;

    glm_vec3_scale(x_axis, -size, x_scaled);
    glm_vec3_add(origin, x_scaled, x_origin);
    glm_vec3_scale(x_scaled, -1, x_scaled);
    glm_vec3_add(origin, x_scaled, x_endpoint);

    for (int y = -size; y <= size; ++y) {
      for (int z = -size; z <= size; ++z) {
        vec3 temp_origin, temp_endpoint;
        vec3 temp_y, temp_z;
        glm_vec3_scale(y_axis, y, temp_y);
        glm_vec3_scale(z_axis, z, temp_z);

        // scale by the basis vectors.
        glm_vec3_add(x_origin, temp_y, temp_origin);
        glm_vec3_add(temp_origin, temp_z, temp_origin);

        glm_vec3_add(x_endpoint, temp_y, temp_endpoint);
        glm_vec3_add(temp_endpoint, temp_z, temp_endpoint);

        im_vector(temp_origin, temp_endpoint, (vec4){1, 0, 0, 1});
      }
    }
  }

  // Draw lines along the y basis
  {
    vec3 y_scaled;
    vec3 y_origin, y_endpoint;

    glm_vec3_scale(y_axis, -size, y_scaled);
    glm_vec3_add(origin, y_scaled, y_origin);
    glm_vec3_scale(y_scaled, -1, y_scaled);
    glm_vec3_add(origin, y_scaled, y_endpoint);

    for (int x = -size; x <= size; ++x) {
      for (int z = -size; z <= size; ++z) {
        vec3 temp_origin, temp_endpoint;
        vec3 temp_x, temp_z;
        glm_vec3_scale(x_axis, x, temp_x);
        glm_vec3_scale(z_axis, z, temp_z);

        glm_vec3_add(y_origin, temp_x, temp_origin);
        glm_vec3_add(temp_origin, temp_z, temp_origin);

        glm_vec3_add(y_endpoint, temp_x, temp_endpoint);
        glm_vec3_add(temp_endpoint, temp_z, temp_endpoint);

        im_vector(temp_origin, temp_endpoint, (vec4){0, 1, 0, 1});
      }
    }
  }

  // Draw lines along the z basis
  {
    vec3 z_scaled;
    vec3 z_origin, z_endpoint;

    glm_vec3_scale(z_axis, -size, z_scaled);
    glm_vec3_add(origin, z_scaled, z_origin);
    glm_vec3_scale(z_scaled, -1, z_scaled);
    glm_vec3_add(origin, z_scaled, z_endpoint);

    for (int x = -size; x <= size; ++x) {
      for (int y = -size; y <= size; ++y) {
        vec3 temp_origin, temp_endpoint;
        vec3 temp_x, temp_y;
        glm_vec3_scale(x_axis, x, temp_x);
        glm_vec3_scale(y_axis, y, temp_y);

        glm_vec3_add(z_origin, temp_x, temp_origin);
        glm_vec3_add(temp_origin, temp_y, temp_origin);

        glm_vec3_add(z_endpoint, temp_x, temp_endpoint);
        glm_vec3_add(temp_endpoint, temp_y, temp_endpoint);

        im_vector(temp_origin, temp_endpoint, (vec4){0, 0, 1, 1});
      }
    }
  }

  for (int x = -size; x <= size; ++x) {
    for (int y = -size; y <= size; ++y) {
      for (int z = -size; z <= size; ++z) {
        // Initialize the point based on grid coordinates
        point[0] = x;
        point[1] = y;
        point[2] = z;

        // Initialize a homogeneous coordinate for transformation
        vec4 hom_point = {point[0], point[1], point[2], 1.0f};

        // Apply transformation
        glm_mat4_mulv(m, hom_point, hom_point);

        // Convert back to Cartesian coordinates from homogeneous coordinates
        transformed_point[0] = hom_point[0] / hom_point[3];
        transformed_point[1] = hom_point[1] / hom_point[3];
        transformed_point[2] = hom_point[2] / hom_point[3];

        // Draw the point
        im_point(hom_point);
      }
    }
  }
}
