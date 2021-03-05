#include <stdio.h>
#include <stdlib.h>

void insertion_sort(int *a, int n) {
  int i, j, t;

  for (i = 1; i < n; i++) {
    for (t = a[i], j = i - 1; j > -1 && a[j] > t; j--) {
      a[j+1] = a[j];
    }
    a[j+1] = t;
  }
}

void insertion_sort_generic(void *data, int n, size_t s,
                            int (*compare)(const void *, const void *)) {
  char *a;
  int i, j;
  void *t;

  a = data;
  t = malloc(s);

  for (i = 1; i < n; i++) {
    for (memcpy(t, a + i * s, s), j = i - 1;
                j > -1 && compare(a + j * s, t) > 0; j--) {
      a[j+1] = a[j];
    }
    a[j+1] = t;
  }
}

int main(int argc, char *argv[]) {
  int a[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
  int i;

  insertion_sort(a, sizeof(a) / sizeof(a[0]));

  for (i = 0; i < 10; i++) {
    printf("%d ", a[i]);
  }

  printf("\n");

  return 0;
}
