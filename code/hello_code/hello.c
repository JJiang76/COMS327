#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  //argv[0] is always the name of the file
  int i;
  int fps;

  printf("The value of argv[2] is %d.\n", atoi(argv[2]));

  if (argc > 1) {
    if (!strcmp("--fps", argv[1])) {
      // string compare - similar to compareTo()
      // use "!" instead of "== 0" - zero has the value false, nonzero has value true
      fps = atoi(argv[2]);
    }
  }

  //usleep(1000000 / fps) // At the end of the loop in assignment 0
  //usleep is in <unistd.h>
  //makes program sleep for a number of microseconds

  for (i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }

  printf("Hello %s!\n", argv[1]);

  return 0;
}
