#include <stdio.h>

void swapint(int *x, int *y) {
  int temp = *x;
  *x = *y;
  *y = temp;
}

/*char *strndup(const char *s, size_t n) {
  int i, size;

  n > strlen(s) ? size = strlen(s): size = n;

  char *ret = (char *) malloc(n + 1);

  for (i = 0; i < size; i++) {
    ret[i] = s[i];
  }
  ret[i] = '\0';

  return ret;
}*/

/*size_t strcspn(const char *s, const char *reject) {
  size_t ret;
  int n;

  for (ret = 0; s[ret]; ret++) {
    for (n = 0; reject[n]; n++) {
      if (s[ret] == reject[n]) {
        return ret;
      }
    }
  }

  return ret;
}*/

int main(int argc, char *argv[]) {
  int x = 0, y = 1;

  void *arr[2] = malloc(50);
  ((int *) arr[0]) = &x;
  ((int *) arr[1]) = &y;

  swapint(&x, &y);
  printf("x=%d, y=%d\n", x, y);

  return 0;
}
