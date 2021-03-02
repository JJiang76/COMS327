#include <stdio.h>

#include "times_table.h"

#define ARRAY_SIZE 10


int main(int argc, char *argv[]) {
  int table[ARRAY_SIZE][ARRAY_SIZE];
  int i, j;

  times_table(table);

  for (j = 0; j < ARRAY_SIZE; j++) {
    for (i = 0; i < ARRAY_SIZE; i++) {
      printf("%3d ", table[j][i]);
    }
    printf("\n");
  }

  return 0;
}

void times_table(int table[ARRAY_SIZE][ARRAY_SIZE]) {
  int i, j;

  for (j = 0; j < ARRAY_SIZE; j++) {
    for (i = 0; i < ARRAY_SIZE; i++) {
      table[j][i] = (j + 1) * (i + 1);
    }
  }
}
