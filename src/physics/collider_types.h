#pragma once

typedef enum ColliderType {
  CL_PILLAR, // shapes a pillar around the object. compares the x and z position
             // between a radius.

  CL_SPHERE, // just checks distance between positions of objects.
  CL_FLOOR,  // if object is below a floor, it's pushed up.
  CL_COUNT,
} ColliderType;

typedef struct PillarColliderData {
  float xz_radius;
} PillarColliderData;

typedef struct SphereColliderData {
  float radius;
} SphereColliderData;

typedef struct FloorColliderData { // height is determined by the physics
                                   // object, not the ColliderData for the
                                   // floor.

  float strength; // how hard should the floor push up?
} FloorColliderData;

typedef void *ColliderDataPtr;

// every gameobject should have an array of colliders.
typedef struct Collider {
  ColliderType type;
  ColliderDataPtr data; // determine the datatype and usage of this field
                        // through the type of Collider.
} Collider;
