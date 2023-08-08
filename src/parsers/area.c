#include "area.h"
#include "core/tile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 256
#define MAX_VAR_NAME 256

// get a representative char from a tile enum variant.
const char tile_ch[TT_COUNT] = {[TT_WALL] = 'X', [TT_FLOOR] = 'O'};

void print_tiledata(TileData t) {
  printf("TILE: [%c], variant num: %d.\n", tile_ch[t.type], t.type);
}

// parse an AreaFile, a format representing a basic area in the game, with
// tiledata and metadata like size and etc
AreaFile *parse_area(const char *area_file_path) {
  FILE *file = fopen(area_file_path, "r");
  if (file == NULL) {
    fprintf(stderr, "Could not open area file. [ %s ]\n", area_file_path);
    return NULL;
  }

  AreaFile *a = (AreaFile *)malloc(sizeof(AreaFile));
  AreaFileSectionType section; // which section are we in?

  // this sucks
  int has_found_size = 0;

  // for the strtok in the actual tile parser.
  int i;
  int tiledata[a->x_size];
  char *token;

  int parsing_line =
      0; // which line of the tile array are we parsing right now?

  char line[MAX_LINE_SIZE];
  char section_name[MAX_VAR_NAME];
  char var_name[MAX_VAR_NAME];
  int var_value;
  // parse line-by-line, until we reach the end of the file.
  while (fgets(line, sizeof(line), file) != NULL) {
    if (line[0] == '#') // lol
      continue;

    if (!has_found_size) {
      // check if we've found the proper size.
      if (a->x_size != 0 && a->y_size != 0) {
        // we've found the size for the first time, malloc the tiledata array.
        a->tiledata = (TileData **)malloc(sizeof(TileData *) *
                                          a->y_size); // y_size number of rows.
        printf("Found the size of the tiledata in this file: "
               "[x_size: %d] "
               "[y_size: %d]\n",
               a->x_size, a->y_size);
        has_found_size = 1;
      }
    }

    // always try to parse a section header, no matter what section state we're
    // in.
    if (sscanf(line, " --- %255[^-] ---", section_name) == 1) {
      printf("Read section: %s\n", section_name);
      if (strncmp("tiledata", section_name, 8) ==
          0) { // check for equality with zero to check equality of the strings.
               // weird!
        section = AFST_TILEDATA;
      } else if (strncmp("metadata", section_name, 8) == 0) {
        section = AFST_METADATA;
      } else {
        fprintf(stderr, "Invalid section name '%s'.\n", section_name);
      }
      continue;
    }

    switch (section) {
      // this will probably fuck up really hard if you set the size after
      // already setting it. don't do that.
    case AFST_METADATA:
      // Try to read a variable assignment in the metadata definition section.
      //
      // TODO: how can we handle more than just ints? if we wanted to make it
      // really simple, we could just do #%d for int, *%s for strings or
      // something, and impose extra conditions on the format. that sucks,
      // though. maybe try to convert the string, and take in the string
      // generically without using sscanf autoconversion?
      if (sscanf(line, "%255s = %d", var_name, &var_value) == 2) {
        if (strncmp("x_size", var_name, 8) == 0) {
          a->x_size = var_value;
        } else if (strncmp("y_size", var_name, 8) == 0) {
          a->y_size = var_value;
        } else {
          fprintf(stderr, "Invalid metadata variable name '%s'. Skipping...\n",
                  var_name);
        }
      }
      break;
    case AFST_TILEDATA:
      // try to read tile definition data.
      if (!has_found_size) {
        fprintf(stderr,
                "You probably forgot to set the grid x_size or y_size "
                "of the area. One of them is set to zero. [x_size: %d] "
                "[y_size: %d]\n",
                a->x_size, a->y_size);
        continue;
      }

      if (parsing_line >= a->y_size) {
        fprintf(stderr,
                "Too many lines in here. Tried to parse the %d'th line, when "
                "there were only %d lines specified.\n",
                parsing_line, a->y_size);
      }

      // mallocing for each line dynamically sucks really really hard. maybe
      // mempool this out?
      TileData *curr_tile_line = (TileData *)malloc(
          sizeof(TileData) * a->x_size); // malloc one line, with width x_size

      i = 0;

      printf("[tiledata] parsing %s", line);

      token = strtok(line, " ");
      while (token != NULL && i < a->x_size) {
        TileData curr_tile;
        // using the enum variant numbers directly as the TileData ids.
        if (sscanf(token, "%d", &curr_tile.type) != 1) {
          fprintf(stderr, "Invalid integer in tile data: %s\n", token);
          break;
        }
        curr_tile_line[i] = curr_tile; // slide it in here.
        token = strtok(NULL, " ");
        i++;
      }

      if (i != a->x_size) {
        fprintf(stderr, "Incorrect number of integers in tile data line\n");
        continue;
      }

      // only update at the end of the loop, a little cleaner this way.
      a->tiledata[parsing_line] = curr_tile_line;

      parsing_line++;

      break;
    default:
      fprintf(stderr, "AreaFile: invalid section %d.\n", section);
      break;
    }
  }

  fclose(file);

  return a;
}

void print_area_file(const AreaFile *areaFile) {
  printf("AreaFile Metadata:\n");
  printf("\tX Size: %u\n", areaFile->x_size);
  printf("\tY Size: %u\n", areaFile->y_size);
  printf("AreaFile Tile Data:\n");

  for (u16 y = 0; y < areaFile->y_size; y++) {
    for (u16 x = 0; x < areaFile->x_size; x++) {
      printf("| %d |", areaFile->tiledata[y][x].type);
    }
    printf("\n");
  }
}
