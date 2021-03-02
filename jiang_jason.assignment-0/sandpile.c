#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "head.h"

#define TOPPLE_HEIGHT 8

int main(int argc, char *argv[]) {
  int table[WORLD_SIZE][WORLD_SIZE], fps = 10, start, i, j;

  for (i = 0; i < WORLD_SIZE; i++) {
    for (j = 0; j < WORLD_SIZE; j++) {
      table[i][j] = 0;
    }
  }


  if (argc > 1) {
    if (!strcmp("--fps", argv[1])) {
      fps = atoi(argv[2]);
      start = 3;
    }
    else {
      start = 1;
    }
    
    int k, x, y, value;
    int count = 0;

    for (k = start; k < argc; k++) {
      switch(count) {
        case 0: y = atoi(argv[k]); count++; break;
        case 1: x = atoi(argv[k]); count++; break;
        case 2: value = atoi(argv[k]);
                if (!(x == WORLD_SIZE/2 && y == WORLD_SIZE/2 && value == -1)) {
                  table[y][x] = value;
                }
                count = 0; break;
      }
    }
  }

  print_table(table);
  printf("\n");

  while (true) {
    drop(table);

    if (table[WORLD_SIZE/2][WORLD_SIZE/2] > TOPPLE_HEIGHT) {
      topple(WORLD_SIZE/2, WORLD_SIZE/2, table);
    }

    print_table(table);

    usleep(1000000 / fps);
    printf("\n");
  }

  return 0;
}

void topple(int x, int y, int table[WORLD_SIZE][WORLD_SIZE]) {
  int i, j;

  table[x][y] = 0;

  for (i = x - 1; i <= x + 1; i++) {

    if (!(i < 0 || i >= WORLD_SIZE)) {

      for (j = y - 1; j <= y + 1; j++) {

        if (!(j < 0 || j >= WORLD_SIZE)) {

          if (table[i][j] != -1) {
            table[i][j]++;
          }

          if (table[i][j] > TOPPLE_HEIGHT) {
            topple(i, j, table);
          }
        }
      }
    }
  }
}

void drop(int table[WORLD_SIZE][WORLD_SIZE]) {
  table[WORLD_SIZE/2][WORLD_SIZE/2]++;
}

void print_table(int table[WORLD_SIZE][WORLD_SIZE]) {
  int i, j;

  for (i = 0; i < WORLD_SIZE; i++) {
    for (j = 0; j < WORLD_SIZE; j++) {
      if (table[i][j] == -1) {
        printf("# ");
      }
      else {
        printf("%d ", table[i][j]);
      }
    }
    printf("\n");
  }
}
