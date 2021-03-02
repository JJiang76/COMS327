#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  //seeds the random generator
  //use time to get a (kinda) random int
  //time() gets the number of seconds since 01/01/1970
  srand(time(NULL));

  printf("%d\n", rand());
  printf("%d\n", rand());
  printf("%d\n", rand());
  printf("%d\n", rand());
  printf("%d\n", rand());

  // 0-9
  printf("%d\n", rand() % 10);

  //max number rand() can generate = 2^31 -1
  printf("RAND_MAX: %d\n", RAND_MAX);

  return 0;
}
