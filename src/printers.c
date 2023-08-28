#include "printers.h"
#include "wjson.h"
#include <stdio.h>

#define INDENT_STRING "  "

#define INDENT                                                                 \
  for (int i = 0; i < indent_level; i++) {                                     \
    printf(INDENT_STRING);                                                     \
  }

void print_int(int i, int indent_level) {
  INDENT
  printf("%d\n", i);
}

void print_vec2(vec2 v, int indent_level) {
  INDENT
  printf("%f %f\n", v[0], v[1]);
}

void print_vec3(vec3 v, int indent_level) {
  INDENT
  printf("%f %f %f\n", v[0], v[1], v[2]);
}

void print_vec4(vec4 v, int indent_level) {
  INDENT
  printf("%f %f %f %f\n", v[0], v[1], v[2], v[3]);
}

// for printing things like lists of parsed indices.
void print_many_int(int *ints, int num_ints, int indent_level) {
  for (int i = 0; i < num_ints; i++) {
    print_int(ints[i], indent_level);
  }
}

void print_many_uint(unsigned int *uints, int num_uints, int indent_level) {
  for (int i = 0; i < num_uints; i++) {
    print_int((int)uints[i], indent_level);
  }
}

void print_many_vec2(float *vec2s, int num_vec2s, int indent_level) {
  for (int i = 0; i < num_vec2s; i += 2) {
    print_vec2(&(vec2s[i]), indent_level);
  }
}

void print_many_vec3(float *vec3s, int num_vec3s, int indent_level) {
  for (int i = 0; i < num_vec3s; i += 3) {
    print_vec3(&(vec3s[i]), indent_level);
  }
}

void print_many_vec4(float *vec4s, int num_vec4s, int indent_level) {
  for (int i = 0; i < num_vec4s; i += 4) {
    print_vec4(&(vec4s[i]), indent_level);
  }
}

void print_mat4(mat4 m, int indent_level) {
  INDENT
  printf("%f %f %f %f\n", m[0][0], m[0][1], m[0][2], m[0][3]);
  INDENT
  printf("%f %f %f %f\n", m[1][0], m[1][1], m[1][2], m[1][3]);
  INDENT
  printf("%f %f %f %f\n", m[2][0], m[2][1], m[2][2], m[2][3]);
  INDENT
  printf("%f %f %f %f\n", m[3][0], m[3][1], m[3][2], m[3][3]);
}

// pass zero for no indentation in the print, for example.
void print_model(const Model *model, int indent_offset) {
  int indent_level = indent_offset;

  if (model == NULL) {
    printf("Model is NULL!\n");
    return;
  }

  INDENT
  printf("Model:\n");
  indent_level++;

  INDENT
  printf("Number of animations: %d\n", model->num_animations);
  INDENT
  printf("Number of roots: %d\n", model->num_roots);
  INDENT
  printf("Number of nodes: %d\n", model->num_nodes);

  INDENT
  printf("Roots: \n");
  indent_level++;
  for (int i = 0; i < model->num_roots; i++) {
    INDENT
    printf("%d\n", model->roots[i]);
  }
  indent_level--;

  INDENT
  printf("Nodes:\n");
  for (int i = 0; i < model->num_nodes; i++) {
    indent_level++;
    INDENT
    printf("Node %d: type: %d, parent: %d, num_children: %d\n", i,
           model->nodes[i].type, model->nodes[i].parent,
           model->nodes[i].num_children);
    indent_level--;
  }
}

const char *get_render_configuration_name(RenderConfiguration rc) {
  switch (rc) {
  case RC_BASIC:
    return "RC_BASIC";
  case RC_HUD:
    return "RC_HUD";
  case RC_MODEL:
    return "RC_MODEL";
  case RC_COUNT:
    return "RC_COUNT";
  }
  return "UNKNOWN";
}

void print_graphics_render(const GraphicsRender *gr, int indent_offset) {
  int indent_level = indent_offset;

  if (gr == NULL) {
    printf("GraphicsRender is NULL!\n");
    return;
  }

  INDENT
  printf("GraphicsRender:\n");
  indent_level++;

  INDENT
  printf("Model matrix: \n");
  indent_level++;
  print_mat4((vec4 *)gr->model, indent_level);
  indent_level--;

  INDENT
  printf("Shader Name: %s\n", gr->shader->name);

  // Add more print statements for the other fields in GraphicsRender as needed
}

#undef INDENT
#undef INDENT_STRING
