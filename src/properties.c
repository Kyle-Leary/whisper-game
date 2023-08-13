#include "properties.h"
#include "string.h"
#include "util.h"
#include <stdio.h>

Properties prop_from_string(const char *prop_string) {
  if (strcmp(prop_string, "translation") == 0) {
    return P_TRANSLATION;
  } else if (strcmp(prop_string, "rotation") == 0) {
    return P_ROTATION;
  } else if (strcmp(prop_string, "scale") == 0) {
    return P_SCALE;
  } else {
    return P_UNKNOWN;
  }
}

void *return_prop_base_ptr(Node *nodes, Target *target, int *prop_sz) {
  Node *target_node = &(nodes[target->node_index]);
  Properties target_prop = prop_from_string(target->property_name);

  switch (target_prop) {
  case P_TRANSLATION: {
    *prop_sz = sizeof(float) * 3;
    return &(target_node->translation);
  } break;
  case P_ROTATION: {
    *prop_sz = sizeof(float) * 4;
    return &(target_node->rotation);
  } break;
  case P_SCALE: {
    *prop_sz = sizeof(float) * 3;
    return &(target_node->scale);
  } break;
  default: {
    fprintf(stderr,
            "ERROR: either the node doesn't contain the property \"%s\", or "
            "it's otherwise invalid. (return_prop_base_ptr())\n",
            target->property_name);
    return NULL;
  } break;
  }
}