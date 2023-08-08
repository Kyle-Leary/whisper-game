#pragma once

#include "core/tile.h"
#include "defines.h"
#include "object.h"

// abstract tile that can be parsed from the tile format. leave room for
// expansion here, don't just make it a u8 or something.
typedef struct TileData {
  TileType type; // has an int enum type, might have some other stuff, like
                 // custom-data per tile? maybe item data?
} TileData;

extern const char tile_ch[TT_COUNT];

void print_tiledata(TileData t);

// use this as the state machine for the parser, we can only be in one section
// at a time.
typedef enum AreaFileSectionType {
  AFST_METADATA,
  AFST_TILEDATA,
  AFST_COUNT
} AreaFileSectionType;

typedef struct AreaFile {
  // metadata of the file
  u16 x_size;
  u16 y_size;

  // table data
  TileData *
      *tiledata; // 2d-array of tiles. store the tiles directly, i think making
                 // this a pointer array would be ugly for the memory layout.
                 // plus, the TileData structure shouldn't really get that big.
} AreaFile;

AreaFile *parse_area(const char *area_file_path);
void print_area_file(const AreaFile *areaFile);
