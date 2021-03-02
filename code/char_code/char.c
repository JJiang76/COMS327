#include <stdio.h>
#include <ctype.h>

char *words[] = {
  "alcohol",
  "bind",
  "cabbage",
  "dredge",
  "ethereum",
  "floral",
  "gucci",
  "hammie",
  "indigo",
  "jorts",
  "kill",
  "luser",
  "money",
  "no",
  "opera",
  "petrichor",
  "queue",
  "ratty",
  "scheaffer",
  "technology",
  "uber",
  "vioin",
  "werewolf",
  "xenomorph",
  "yeet",
  "zing"
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Please supply a letter!\n");

    return -1;
  }

  printf("%s beginds with %c\n", words[tolower(argv[1][0]) - 'a'], argv[1][0]);

  return 0;
}
