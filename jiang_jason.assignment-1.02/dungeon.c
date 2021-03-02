#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <endian.h>
#include <string.h>

#include "dungeon.h"

#define MIN_ROOMS 6
#define MAX_ROOMS 8
#define MIN_YSIZE 3
#define MIN_XSIZE 4
#define MAX_YSIZE 8
#define MAX_XSIZE 9
#define Y_DUNGEON 21
#define X_DUNGEON 80

typedef struct room {
  int xpos;
  int ypos;
  int xsize;
  int ysize;
} room_t;

typedef struct pc {
  int x, y;
} pc_t;

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int dungeon[21][80];
  pc_t pc;
  room_t *rooms;
  int swload, swsave;
  swload = swsave = 0;

  char *home = getenv("HOME");
  char *gamedir = ".rlg327";
  char *savefile = "dungeon";
  char *path = malloc(strlen(home) + strlen(gamedir) + strlen(savefile) + 3);

  sprintf(path, "%s/%s/%s", home, gamedir, savefile);


  if (argc > 1) {
    int i;

    for (i = 1; i < argc; i++) {
      if (!strcmp("--load", argv[i])) {
        swload = 1;
      }
      else if (!strcmp("--save", argv[i])) {
        swsave = 1;
      }
    }
  }

  setup(dungeon);

  if (swload) {   //load
    FILE *f;
    if (!(f = fopen(path, "r"))) {
      fprintf(stderr, "Failed to open file for reading\n");

      return -1;
    }

    char semantic[13];
    semantic[12] = '\0';
    fread(semantic, 1, 12, f);

    uint32_t version;
    fread(&version, 4, 1, f);
    version = be32toh(version);

    uint32_t size;
    fread(&size, 4, 1, f);
    size = be32toh(size);

    fread(&pc.x, 1, 1, f);
    fread(&pc.y, 1, 1, f);

    int i, j;

    for (i = 0; i < 21; i++) {
      for (j = 0; j < 80; j++) {
        fread(&dungeon[i][j], 1, 1, f);
      }
    }

    uint16_t num_rooms;
    fread(&num_rooms, 2, 1, f);
    num_rooms = be16toh(num_rooms);
    rooms = malloc(num_rooms * sizeof(room_t));

    int k;
    for (k = 0; k < num_rooms; k++) {
      fread(&rooms[k].xpos, 1, 1, f);
      fread(&rooms[k].ypos, 1, 1, f);
      fread(&rooms[k].xsize, 1, 1, f);
      fread(&rooms[k].ysize, 1, 1, f);

      int i, j;

      for (j = rooms[k].ypos; j < rooms[k].ysize + rooms[k].ypos; j++) {
        for (i = rooms[k].xpos; i < rooms[k].xsize + rooms[k].xpos; i++) {
          dungeon[j][i] = -1;
        }
      }
    }

    uint16_t upstairs, downstairs;
    fread(&upstairs, 2, 1, f);
    upstairs = be16toh(upstairs);

    int x, y;

    for (i = 0; i < upstairs; i++) {
      fread(&x, 1, 1, f);
      fread(&y, 1, 1, f);
      dungeon[y][x] = -3;
    }

    fread(&downstairs, 2, 1, f);
    downstairs = be16toh(downstairs);

    for (i = 0; i < downstairs; i++) {
      fread(&x, 1, 1, f);
      fread(&y, 1, 1, f);
      dungeon[y][x] = -4;
    }

    //place pc
    dungeon[pc.y][pc.x] = -2;

    fclose(f);
  }

  else { //dont load - default
    rooms = generate_dungeon(dungeon);
  }

  print_dungeon(dungeon);

  if (swsave) {
    FILE *f;
    if (!(f = fopen(path, "w"))) {
      fprintf(stderr, "Failed to open file for writing\n");

      return -1;
    }

    char *semantic = "RLG327-S2021";
    int i;
    for (i = 0; i < strlen(semantic); i++) {
      fwrite(&semantic[i], 1, 1, f);
    }

    uint32_t version = 1;
    version = htobe32(version);
    fwrite(&version, 4, 1, f);

    uint32_t size;
    size = htobe32(size);
    fwrite(&size, 4, 1, f);

    //pc 
    fwrite(&rooms[0].xpos, 1, 1, f);
    fwrite(&rooms[0].ypos, 1, 1, f);
  }


  return 0;
}

void setup(int dungeon[21][80]) {
  srand(time(NULL));

  int i, j;

  for (i = 0; i < 21; i += 20) {
    for (j = 0; j < 80; j++) {
      dungeon[i][j] = 255;
    }
  }

  for (i = 0; i < 80; i += 79) {
    for (j = 0; j < 21; j++) {
      dungeon[j][i] = 255;
    }
  }

  //should add more randomness somewhere
  for (i = 1; i < 20; i++) {
    for (j = 1; j < 79; j++) {
      dungeon[i][j] = (rand() % 254) + 1;
    }
  }
}

room_t *generate_dungeon(int dungeon[21][80]) {
  int i, num_rooms = (rand() % (MAX_ROOMS - MIN_ROOMS + 1)) + MIN_ROOMS;
  room_t *rooms = malloc(MAX_ROOMS * sizeof(room_t));

  for (i = 0; i < num_rooms; i++) {
    bool go = true, success;

    while (go) {
      success = true;

      rooms[i].xsize = (rand() % (MAX_XSIZE - MIN_XSIZE + 1)) + MIN_XSIZE;
      rooms[i].ysize = (rand() % (MAX_YSIZE - MIN_YSIZE + 1)) + MIN_YSIZE;
      rooms[i].xpos = (rand() % (X_DUNGEON - 1 - MAX_YSIZE)) + 1;
      rooms[i].ypos = (rand() % (Y_DUNGEON - 1 - MAX_YSIZE)) + 1;

      int j, k;

      //check collisions
      for (j = rooms[i].ypos; j < rooms[i].ysize + rooms[i].ypos; j++) {

        if (dungeon[j][rooms[i].xpos - 1] == -1) { //left edge
          j = rooms[i].ysize + rooms[i].ypos + 1;
          success = false;
          break;
        }

        if (dungeon[j][rooms[i].xsize + rooms[i].xpos] == -1) { //top edge
          j = rooms[i].ysize + rooms[i].ypos + 1;
          success = false;
          break;
        }

        for (k = rooms[i].xpos; k < rooms[i].xsize + rooms[i].xpos; k++) {

          if (j == rooms[i].ypos) { //top edge
            if (dungeon[j - 1][k] == -1) {
              j = rooms[i].ysize + rooms[i].ypos + 1;
              k = rooms[i].xsize + rooms[i].xpos + 1;
              success = false;
              break;
            }
          }

          if (j == rooms[i].ysize + rooms[i].ypos - 1) { //bottom edge
            if (dungeon[j + 1][k] == -1) {
              j = rooms[i].ysize + rooms[i].ypos + 1;
              k = rooms[i].xsize + rooms[i].xpos + 1;
              success = false;
              break;
            }
          }

          if (dungeon[j][k] == -1 || dungeon[j][k] == 255) { //collision
            j = rooms[i].ysize + rooms[i].ypos + 1;
            k = rooms[i].xsize + rooms[i].xpos + 1;
            success = false;
            break;
          }
        }
      }

      //check corners
      if (dungeon[rooms[i].ypos - 1][rooms[i].xpos - 1] == -1
        || dungeon[rooms[i].ypos + rooms[i].ysize + 1][rooms[i].xpos - 1] == -1
        || dungeon[rooms[i].ypos - 1][rooms[i].xpos + rooms[i].xsize + 1] == -1
        || dungeon[rooms[i].ypos + rooms[i].ysize + 1][rooms[i].xpos + rooms[i].xsize + 1] == -1) {
          success = false;
        }

        if (success) {
          for (j = rooms[i].ypos; j < rooms[i].ysize + rooms[i].ypos; j++) {
            for (k = rooms[i].xpos; k < rooms[i].xsize + rooms[i].xpos; k++) {
              dungeon[j][k] = -1;
            }
          }
          go = false;
        }
      }
    }

    //connect rooms
    for (i = 0; i < num_rooms - 1; i++) {
      int y0 = rooms[i].ypos;
      int x0 = rooms[i].xpos;
      int y1 = rooms[i+1].ypos;
      int x1 = rooms[i+1].xpos;

      int y_dir = y0 < y1 ? 1 : -1;
      int x_dir = x0 < x1 ? 1 : -1;
      int y, x;

      for (y = y0; y_dir == 1 ? y < y1 : y > y1; y += y_dir) {
        if (dungeon[y][x0] != -1) {
          dungeon[y][x0] = 0;
        }
      }

      for (x = x0; x_dir == 1 ? x < x1 : x > x1; x += x_dir) {
        if (dungeon[y1][x] != -1) {
          dungeon[y1][x] = -0;
        }
      }
    }

    //create cycle for final room
    int y0 = rooms[i].ypos;
    int x0 = rooms[i].xpos;
    int y1 = rooms[0].ypos;
    int x1 = rooms[0].xpos;

    int y_dir = y0 < y1 ? 1 : -1;
    int x_dir = x0 < x1 ? 1 : -1;
    int y, x;

    for (y = y0; y_dir == 1 ? y < y1 : y > y1; y += y_dir) {
      if (dungeon[y][x0] != -1) {
        dungeon[y][x0] = 0;
      }
    }

    for (x = x0; x_dir == 1 ? x < x1 : x > x1; x += x_dir) {
      if (dungeon[y1][x] != -1) {
        dungeon[y1][x] = 0;
      }
    }

    //add random staircases
    int s = 0, stairs = (rand() % 3) + 2;

    while (s < stairs / 2) {
      int n = (rand() % 20) + 1, m = (rand() % 79) + 1;

      if (dungeon[n][m] == 0 || dungeon[n][m] == -1) {
        dungeon[n][m] = -3; //up
        s++;
      }
    }

    while (s < stairs) {
      int n = (rand() % 20) + 1, m = (rand() % 79) + 1;

      if (dungeon[n][m] == 0 || dungeon[n][m] == -1) {
        dungeon[n][m] = -4; //down
        s++;
      }
    }

    //place pc in room[0] top left
    pc_t pc;
    pc.x = rooms[0].xpos;
    pc.y = rooms[0].ypos;
    dungeon[pc.y][pc.x] = -2;

    return rooms;
  }

void print_dungeon(int dungeon[21][80]) {
    int i, j;

    for (i = 0; i < 21; i++) {
      for (j = 0; j < 80; j++) {
        if (dungeon[i][j] == 255) {
          printf(" "); //border
        }
        else if (dungeon[i][j] == 0) {
          printf("#"); //corridor
        }
        else if (dungeon[i][j] == -1) {
          printf("."); //floor
        }
        else if (dungeon[i][j] == -2) {
          printf("@"); //player
        }
        else if (dungeon[i][j] == -3) {
          printf("<"); //up
        }
        else if (dungeon[i][j] == -4) {
          printf(">"); //down
        }
        else {
          printf(" ");
        }
      }
      printf("\n");
    }
  }
