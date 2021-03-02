#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "dungeon.h"

#define MIN_ROOMS 6
#define MAX_ROOMS 8
#define MIN_YSIZE 3
#define MIN_XSIZE 4
#define MAX_YSIZE 8
#define MAX_XSIZE 9
#define Y_DUNGEON 21
#define X_DUNGEON 80

struct room {
  int xpos;
  int ypos;
  int xsize;
  int ysize;
};

int main(int argc, char *argv[]) {
  srand(time(NULL));

  int dungeon[21][80];
  struct room rooms[MAX_ROOMS];

  setup(dungeon);

  generate_dungeon(dungeon);

  print_dungeon(dungeon);

  return 0;
}

void setup(int dungeon[21][80]) {
  srand(time(NULL));

  int i, j;

  for (i = 0; i < 21; i += 20) {
    for (j = 0; j < 80; j++) {
      dungeon[i][j] = -1;
    }
  }

  for (i = 0; i < 80; i += 79) {
    for (j = 0; j < 21; j++) {
      dungeon[j][i] = -1;
    }
  }

  //should add more randomness somewhere
  for (i = 1; i < 20; i++) {
    for (j = 1; j < 79; j++) {
      dungeon[i][j] = (rand() % 255) + 1;
    }
  }
}

void generate_dungeon(int dungeon[21][80]) {
  int i, num_rooms = (rand() % (MAX_ROOMS - MIN_ROOMS + 1)) + MIN_ROOMS;

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

        if (dungeon[j][rooms[i].xpos - 1] == 0) { //left edge
          j = rooms[i].ysize + rooms[i].ypos + 1;
          success = false;
          break;
        }

        if (dungeon[j][rooms[i].xsize + rooms[i].xpos] == 0) { //top edge
          j = rooms[i].ysize + rooms[i].ypos + 1;
          success = false;
          break;
        }

        for (k = rooms[i].xpos; k < rooms[i].xsize + rooms[i].xpos; k++) {

          if (j == rooms[i].ypos) { //top edge
            if (dungeon[j - 1][k] == 0) {
              j = rooms[i].ysize + rooms[i].ypos + 1;
              k = rooms[i].xsize + rooms[i].xpos + 1;
              success = false;
              break;
            }
          }

          if (j == rooms[i].ysize + rooms[i].ypos - 1) { //bottom edge
            if (dungeon[j + 1][k] == 0) {
              j = rooms[i].ysize + rooms[i].ypos + 1;
              k = rooms[i].xsize + rooms[i].xpos + 1;
              success = false;
              break;
            }
          }

          if (dungeon[j][k] == 0 || dungeon[j][k] == -1) { //collision
            j = rooms[i].ysize + rooms[i].ypos + 1;
            k = rooms[i].xsize + rooms[i].xpos + 1;
            success = false;
            break;
          }
        }
      }

      //check corners
      if (dungeon[rooms[i].ypos - 1][rooms[i].xpos - 1] == 0
        || dungeon[rooms[i].ypos + rooms[i].ysize + 1][rooms[i].xpos - 1] == 0
        || dungeon[rooms[i].ypos - 1][rooms[i].xpos + rooms[i].xsize + 1] == 0
        || dungeon[rooms[i].ypos + rooms[i].ysize + 1][rooms[i].xpos + rooms[i].xsize + 1] == 0) {
          success = false;
        }

        if (success) {
          for (j = rooms[i].ypos; j < rooms[i].ysize + rooms[i].ypos; j++) {
            for (k = rooms[i].xpos; k < rooms[i].xsize + rooms[i].xpos; k++) {
              dungeon[j][k] = 0;
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
        if (dungeon[y][x0] != 0) {
          dungeon[y][x0] = -2;
        }
      }

      for (x = x0; x_dir == 1 ? x < x1 : x > x1; x += x_dir) {
        if (dungeon[y1][x] != 0) {
          dungeon[y1][x] = -2;
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
      if (dungeon[y][x0] != 0) {
        dungeon[y][x0] = -2;
      }
    }

    for (x = x0; x_dir == 1 ? x < x1 : x > x1; x += x_dir) {
      if (dungeon[y1][x] != 0) {
        dungeon[y1][x] = -2;
      }
    }

    //add random staircases
    int s = 0, stairs = (rand() % 3) + 2;

    while (s < stairs / 2) {
      int n = (rand() % 20) + 1, m = (rand() % 79) + 1;

      if (dungeon[n][m] == 0 || dungeon[n][m] == -2) {
        dungeon[n][m] = -3; //up
        s++;
      }
    }

    while (s < stairs) {
      int n = (rand() % 20) + 1, m = (rand() % 79) + 1;

      if (dungeon[n][m] == 0 || dungeon[n][m] == -2) {
        dungeon[n][m] = -4; //down
        s++;
      }
    }
  }

  void print_dungeon(int dungeon[21][80]) {
    int i, j;

    for (i = 0; i < 21; i++) {
      for (j = 0; j < 80; j++) {
        if (dungeon[i][j] == -1) {
          printf("%d", 9); //border
        }
        else if (dungeon[i][j] == 0) {
          printf("."); //floor
        }
        else if (dungeon[i][j] == -2) {
          printf("#"); //corridor
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
