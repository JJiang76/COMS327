#include <stdio.h>

void swap_wrong(int x, int y) {
  int tmp;

  tmp = x;
  x = y;
  y = tmp;

  printf("In swap_wrong, x: %d, y = %d\n", x, y);
}

void swap_right(int *x, int *y) {
  int tmp;

//dereference
  tmp = *x;
  *x = *y;
  *y = tmp;

  printf("In swap_right, x: %d, y = %d\n", *x, *y);
}

int main (int argc, char *argv[]) {
  int x = 0, y = 1;

  printf("x: %d, y = %d\n", x, y);

  swap_wrong(x, y);

  printf("x: %d, y = %d\n", x, y);

  swap_right(&x, &y);
  //make x, y pointers since the function takes pointers

  printf("x: %d, y = %d\n", x, y);

  return 0;
}
