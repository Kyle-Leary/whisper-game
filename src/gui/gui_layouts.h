#pragma once

#include "helper_math.h"
#include <stdint.h>

typedef enum LayoutType {
  LAYOUT_VERTICAL,
  LAYOUT_HORIZONTAL,
  LAYOUT_COUNT,
} LayoutType;

#define LAYOUT_FIELDS                                                          \
  LayoutType type;                                                             \
  float padding;                                                               \
  float margin;

typedef struct Layout {
  LAYOUT_FIELDS
} Layout;

typedef struct LayoutVertical {
  LAYOUT_FIELDS
} LayoutVertical;

typedef struct LayoutHorizontal {
  LAYOUT_FIELDS
} LayoutHorizontal;

extern int layout_sizes[LAYOUT_COUNT];

// avoid malloc by storing raw data larger than the largest possible layout.
// no pointer chasing bullshit this time!!!
#define LAYOUT_BUF_SZ                                                          \
  (MAX(MAX(sizeof(Layout), sizeof(LayoutVertical)), sizeof(LayoutHorizontal)))

// don't call these directly if you're using the gui subsystem. these are
// internal.
void layout_push(Layout *layout);
void layout_pop();
void layout_accept_new(AABB *aabb);
void layout_reset();
void layout_internal_push();

void layout_init();
void layout_clean();
