#include "parsers/gltf/gltf_parse.h"
#include "path.h"

int main(int argc, char *argv[]) {
  GLTFFile *file = gltf_parse(MODEL_PATH("final_boss.glb"));
  return 0;
}
