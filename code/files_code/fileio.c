#include <stdio.h>

type enum action {
  read_text,
  write_text,
  read_binary,
  write_binary
} action_t;

/* Valid switches to main are:
 * -wt Write text
 * -wb Write binary
 * -rt Read text
 * -rb Read binary
 */


int main(int argc, char *argv[]) {
  FILE *f;

  //anonymous struct - basically one time use struct
  struct {
    int i;
    int j;
  } s = {1, 2}; //instance of struct called s with values 1, 2 respectively

  action_t action;

  if (argc != 2) { //no arguments
    fprintf(stderr, "Usage: %s <-wt|-wb|-rt|-rb>\n", argv[0]);

    return -1;
  }

  //check for switches
  if (!strcmp(argv[1], "-wt")) {
    action = write_text;
  }
  else if (!strcmp(argv[1]. "-wb")) {
    action = write_binary;
  }
  else if (!strcmp(argv[1]. "-rt")) {
    action = read_text;
  }
  else if (!strcmp(argv[1]. "-rb")) {
    action = read_binary;
  }
  else {
    fprintf(stderr, "Usage: %s <-wt|-wb|-rt|-rb>\n", argv[0]);

    return -1;
  }

  switch (action) {
    case write_text:
      //fopen(file_name, flag) - w = "write", also has r = "read" etc
      // write = clears file and begins to write (w)
      // append = appends to end of file (a)
      //if f == NULL, then same as 0; !0 = 1; goes inside if statement
      if (!(f = fopen("text_file", "w"))) {
        fprintf(stderr, "Failed to open file for writing");

        return -1; // return nonzero for error - program failed; -1 is standard
      }

      fprintf(f, "%d %d\n", s.i, s.j);
      fclose(f);
      break;

    case read_text:
      //reading
      if (!(f = fopen("text_file", "r"))) {
        fprintf(stderr, "Failed to open file for reading");

        return -1; // return nonzero for error - program failed; -1 is standard
      }
      s.i = s.j = 0;

      //reading - taking values from file and storing them in s.i, s.j
      //need '&' because we want to change values
      fscanf(f, "%d %d\n", &s.i, &s.j);
      printf("%d %d\n", s.i, s.j);
      fclose(f);
      break;

    case write_binary:
      if (!(f = fopen("text_file", "w"))) {
        fprintf(stderr, "Failed to open file for writing");

        return -1;
      }

      fwrite(f, "%d %d\n", s.i, s.j);
      fclose(f);
      break;

    case read_binary:
      break;
  }


  return 0;
}
