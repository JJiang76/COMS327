#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <endian.h>
#include <string.h>

#include "dungeon.h"

#define MIN_ROOMS 6
#define MAX_ROOMS 8
#define MIN_YSIZE 3
#define MIN_XSIZE 4
#define MAX_YSIZE 8
#define MAX_XSIZE 9
#define Y_DUNGEON 21
#define X_DUNGEON 80

typedef struct room {
  int xpos;
  int ypos;
  int xsize;
  int ysize;
} room_t;

typedef struct pc {
  int x, y;
} pc_t;

typedef struct dungeon {
  uint16_t num_rooms;
  room_t *rooms;
  int *map[Y_DUNGEON][X_DUNGEON];
  pc_t pc;
} dungeon_t;

int write_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fwrite(&d->hardness[y][x], sizeof (unsigned char), 1, f);
    }
  }

  return 0;
}

int write_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint16_t p;

  p = htobe16(d->num_rooms);
  fwrite(&p, 2, 1, f);
  for (i = 0; i < d->num_rooms; i++) {
    /* write order is xpos, ypos, width, height */
    p = d->rooms[i].position[dim_x];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].position[dim_y];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size[dim_x];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size[dim_y];
    fwrite(&p, 1, 1, f);
  }

  return 0;
}

uint16_t count_up_stairs(dungeon_t *d)
{
  uint32_t x, y;
  uint16_t i;

  for (i = 0, y = 1; y < DUNGEON_Y - 1; y++) {
    for (x = 1; x < DUNGEON_X - 1; x++) {
      if (mapxy(x, y) == ter_stairs_up) {
        i++;
      }
    }
  }

  return i;
}

uint16_t count_down_stairs(dungeon_t *d)
{
  uint32_t x, y;
  uint16_t i;

  for (i = 0, y = 1; y < DUNGEON_Y - 1; y++) {
    for (x = 1; x < DUNGEON_X - 1; x++) {
      if (mapxy(x, y) == ter_stairs_down) {
        i++;
      }
    }
  }

  return i;
}

int write_stairs(dungeon_t *d, FILE *f)
{
  uint16_t num_stairs;
  uint8_t x, y;

  num_stairs = htobe16(count_up_stairs(d));
  fwrite(&num_stairs, 2, 1, f);
  for (y = 1; y < DUNGEON_Y - 1 && num_stairs; y++) {
    for (x = 1; x < DUNGEON_X - 1 && num_stairs; x++) {
      if (mapxy(x, y) == ter_stairs_up) {
        num_stairs--;
        fwrite(&x, 1, 1, f);
        fwrite(&y, 1, 1, f);
      }
    }
  }

  num_stairs = htobe16(count_down_stairs(d));
  fwrite(&num_stairs, 2, 1, f);
  for (y = 1; y < DUNGEON_Y - 1 && num_stairs; y++) {
    for (x = 1; x < DUNGEON_X - 1 && num_stairs; x++) {
      if (mapxy(x, y) == ter_stairs_down) {
        num_stairs--;
        fwrite(&x, 1, 1, f);
        fwrite(&y, 1, 1, f);
      }
    }
  }

  return 0;
}

uint32_t calculate_dungeon_size(dungeon_t *d)
{
  /* Per the spec, 1708 is 12 byte semantic marker + 4 byte file verion + *
   * 4 byte file size + 2 byte PC position + 1680 byte hardness array +   *
   * 2 byte each number of rooms, number of up stairs, number of down     *
   * stairs.                                                              */
  return (1708 + (d->num_rooms * 4) +
          (count_up_stairs(d) * 2)  +
          (count_down_stairs(d) * 2));
}

int makedirectory(char *dir)
{
  char *slash;

  for (slash = dir + strlen(dir); slash > dir && *slash != '/'; slash--)
    ;

  if (slash == dir) {
    return 0;
  }

  if (mkdir(dir, 0700)) {
    if (errno != ENOENT && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
    if (*slash != '/') {
      return 1;
    }
    *slash = '\0';
    if (makedirectory(dir)) {
      *slash = '/';
      return 1;
    }

    *slash = '/';
    if (mkdir(dir, 0700) && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
  }

  return 0;
}

int write_dungeon(dungeon_t *d, char *file)
{
  char *home;
  char *filename;
  FILE *f;
  size_t len;
  uint32_t be32;

  if (!file) {
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/", home, SAVE_DIR);
    makedirectory(filename);
    strcat(filename, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "w"))) {
      perror(filename);
      free(filename);

      return 1;
    }
    free(filename);
  } else {
    if (!(f = fopen(file, "w"))) {
      perror(file);
      exit(-1);
    }
  }

  /* The semantic, which is 6 bytes, 0-11 */
  fwrite(DUNGEON_SAVE_SEMANTIC, 1, sizeof (DUNGEON_SAVE_SEMANTIC) - 1, f);

  /* The version, 4 bytes, 12-15 */
  be32 = htobe32(DUNGEON_SAVE_VERSION);
  fwrite(&be32, sizeof (be32), 1, f);

  /* The size of the file, 4 bytes, 16-19 */
  be32 = htobe32(calculate_dungeon_size(d));
  fwrite(&be32, sizeof (be32), 1, f);

  /* The PC position, 2 bytes, 20-21 */
  fwrite(&d->pc[dim_x], 1, 1, f);
  fwrite(&d->pc[dim_y], 1, 1, f);

  /* The dungeon map, 1680 bytes, 22-1702 */
  write_dungeon_map(d, f);

  /* The rooms, num_rooms * 4 bytes, 1703-end */
  write_rooms(d, f);

  /* And the stairs */
  write_stairs(d, f);

  fclose(f);

  return 0;
}

int read_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fread(&d->hardness[y][x], sizeof (d->hardness[y][x]), 1, f);
      if (d->hardness[y][x] == 0) {
        /* Mark it as a corridor.  We can't recognize room cells until *
         * after we've read the room array, which we haven't done yet. */
        d->map[y][x] = ter_floor_hall;
      } else if (d->hardness[y][x] == 255) {
        d->map[y][x] = ter_wall_immutable;
      } else {
        d->map[y][x] = ter_wall;
      }
    }
  }


  return 0;
}

int read_stairs(dungeon_t *d, FILE *f)
{
  uint16_t num_stairs;
  uint8_t x, y;

  fread(&num_stairs, 2, 1, f);
  num_stairs = be16toh(num_stairs);
  for (; num_stairs; num_stairs--) {
    fread(&x, 1, 1, f);
    fread(&y, 1, 1, f);
    mapxy(x, y) = ter_stairs_up;
  }

  fread(&num_stairs, 2, 1, f);
  num_stairs = be16toh(num_stairs);
  for (; num_stairs; num_stairs--) {
    fread(&x, 1, 1, f);
    fread(&y, 1, 1, f);
    mapxy(x, y) = ter_stairs_down;
  }
  return 0;
}

int read_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint32_t x, y;
  uint16_t p;

  fread(&p, 2, 1, f);
  d->num_rooms = be16toh(p);
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0; i < d->num_rooms; i++) {
    fread(&p, 1, 1, f);
    d->rooms[i].position[dim_x] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].position[dim_y] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size[dim_x] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size[dim_y] = p;

    if (d->rooms[i].size[dim_x] < 1             ||
        d->rooms[i].size[dim_y] < 1             ||
        d->rooms[i].size[dim_x] > DUNGEON_X - 1 ||
        d->rooms[i].size[dim_y] > DUNGEON_X - 1) {
      fprintf(stderr, "Invalid room size in restored dungeon.\n");

      exit(-1);
    }

    if (d->rooms[i].position[dim_x] < 1                                       ||
        d->rooms[i].position[dim_y] < 1                                       ||
        d->rooms[i].position[dim_x] > DUNGEON_X - 1                           ||
        d->rooms[i].position[dim_y] > DUNGEON_Y - 1                           ||
        d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x] > DUNGEON_X - 1 ||
        d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x] < 0             ||
        d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y] > DUNGEON_Y - 1 ||
        d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y] < 0)             {
      fprintf(stderr, "Invalid room position in restored dungeon.\n");

      exit(-1);
    }


    /* After reading each room, we need to reconstruct them in the dungeon. */
    for (y = d->rooms[i].position[dim_y];
         y < d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y];
         y++) {
      for (x = d->rooms[i].position[dim_x];
           x < d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x];
           x++) {
        mapxy(x, y) = ter_floor_room;
      }
    }
  }

  return 0;
}

int read_dungeon(dungeon_t *d, char *file)
{
  char semantic[sizeof (DUNGEON_SAVE_SEMANTIC)];
  uint32_t be32;
  FILE *f;
  char *home;
  size_t len;
  char *filename;
  struct stat buf;

  if (!file) {
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/%s", home, SAVE_DIR, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "r"))) {
      perror(filename);
      free(filename);
      exit(-1);
    }

    if (stat(filename, &buf)) {
      perror(filename);
      exit(-1);
    }

    free(filename);
  } else {
    if (!(f = fopen(file, "r"))) {
      perror(file);
      exit(-1);
    }
    if (stat(file, &buf)) {
      perror(file);
      exit(-1);
    }
  }

  d->num_rooms = 0;

  fread(semantic, sizeof (DUNGEON_SAVE_SEMANTIC) - 1, 1, f);
  semantic[sizeof (DUNGEON_SAVE_SEMANTIC) - 1] = '\0';
  if (strncmp(semantic, DUNGEON_SAVE_SEMANTIC,
	      sizeof (DUNGEON_SAVE_SEMANTIC) - 1)) {
    fprintf(stderr, "Not an RLG327 save file.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (be32toh(be32) != 0) { /* Since we expect zero, be32toh() is a no-op. */
    fprintf(stderr, "File version mismatch.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (buf.st_size != be32toh(be32)) {
    fprintf(stderr, "File size mismatch.\n");
    exit(-1);
  }

  fread(&d->pc[dim_x], 1, 1, f);
  fread(&d->pc[dim_y], 1, 1, f);

  read_dungeon_map(d, f);

  read_rooms(d, f);

  read_stairs(d, f);

  fclose(f);

  return 0;
}


void setup(dungeon_t *d) {
  srand(time(NULL));

  int i, j;

  for (i = 0; i < 21; i += 20) {
    for (j = 0; j < 80; j++) {
      d->map[i][j] = 255;
    }
  }

  for (i = 0; i < 80; i += 79) {
    for (j = 0; j < 21; j++) {
      d->map[j][i] = 255;
    }
  }

  //should add more randomness somewhere
  for (i = 1; i < 20; i++) {
    for (j = 1; j < 79; j++) {
      d->map[i][j] = (rand() % 254) + 1;
    }
  }
}

void generate_dungeon(dungeon_t *d) {
  int i, d->num_rooms = (rand() % (MAX_ROOMS - MIN_ROOMS + 1)) + MIN_ROOMS;
  d->rooms = malloc(MAX_ROOMS * sizeof(room_t));

  for (i = 0; i < d->d->num_rooms; i++) {
    bool go = true, success;

    while (go) {
      success = true;

      d->rooms[i].xsize = (rand() % (MAX_XSIZE - MIN_XSIZE + 1)) + MIN_XSIZE;
      d->rooms[i].ysize = (rand() % (MAX_YSIZE - MIN_YSIZE + 1)) + MIN_YSIZE;
      d->rooms[i].xpos = (rand() % (X_DUNGEON - 1 - MAX_YSIZE)) + 1;
      d->rooms[i].ypos = (rand() % (Y_DUNGEON - 1 - MAX_YSIZE)) + 1;

      int j, k;

      //check collisions
      for (j = d->rooms[i].ypos; j < d->rooms[i].ysize + d->rooms[i].ypos; j++) {

        if (d->map[j][d->rooms[i].xpos - 1] == -1) { //left edge
          j = d->rooms[i].ysize + d->rooms[i].ypos + 1;
          success = false;
          break;
        }

        if (d->map[j][d->rooms[i].xsize + d->rooms[i].xpos] == -1) { //top edge
          j = d->rooms[i].ysize + d->rooms[i].ypos + 1;
          success = false;
          break;
        }

        for (k = d->rooms[i].xpos; k < d->rooms[i].xsize + d->rooms[i].xpos; k++) {

          if (j == d->rooms[i].ypos) { //top edge
            if (d->map[j - 1][k] == -1) {
              j = d->rooms[i].ysize + d->rooms[i].ypos + 1;
              k = d->rooms[i].xsize + d->rooms[i].xpos + 1;
              success = false;
              break;
            }
          }

          if (j == d->rooms[i].ysize + d->rooms[i].ypos - 1) { //bottom edge
            if (d->map[j + 1][k] == -1) {
              j = d->rooms[i].ysize + d->rooms[i].ypos + 1;
              k = d->rooms[i].xsize + d->rooms[i].xpos + 1;
              success = false;
              break;
            }
          }

          if (d->map[j][k] == -1 || d->map[j][k] == 255) { //collision
            j = d->rooms[i].ysize + d->rooms[i].ypos + 1;
            k = d->rooms[i].xsize + d->rooms[i].xpos + 1;
            success = false;
            break;
          }
        }
      }

      //check corners
      if (d->map[d->rooms[i].ypos - 1][d->rooms[i].xpos - 1] == -1
        || d->map[d->rooms[i].ypos + d->rooms[i].ysize + 1][d->rooms[i].xpos - 1] == -1
        || d->map[d->rooms[i].ypos - 1][d->rooms[i].xpos + d->rooms[i].xsize + 1] == -1
        || d->map[d->rooms[i].ypos + d->rooms[i].ysize + 1][d->rooms[i].xpos + d->rooms[i].xsize + 1] == -1) {
          success = false;
        }

        if (success) {
          for (j = d->rooms[i].ypos; j < d->rooms[i].ysize + d->rooms[i].ypos; j++) {
            for (k = d->rooms[i].xpos; k < d->rooms[i].xsize + d->rooms[i].xpos; k++) {
              d->map[j][k] = -1;
            }
          }
          go = false;
        }
      }
    }

    //connect rooms
    for (i = 0; i < d->d->num_rooms - 1; i++) {
      int y0 = d->rooms[i].ypos;
      int x0 = d->rooms[i].xpos;
      int y1 = d->rooms[i+1].ypos;
      int x1 = d->rooms[i+1].xpos;

      int y_dir = y0 < y1 ? 1 : -1;
      int x_dir = x0 < x1 ? 1 : -1;
      int y, x;

      for (y = y0; y_dir == 1 ? y < y1 : y > y1; y += y_dir) {
        if (d->map[y][x0] != -1) {
          d->map[y][x0] = 0;
        }
      }

      for (x = x0; x_dir == 1 ? x < x1 : x > x1; x += x_dir) {
        if (d->map[y1][x] != -1) {
          d->map[y1][x] = -0;
        }
      }
    }

    //create cycle for final room
    int y0 = d->rooms[i].ypos;
    int x0 = d->rooms[i].xpos;
    int y1 = d->rooms[0].ypos;
    int x1 = d->rooms[0].xpos;

    int y_dir = y0 < y1 ? 1 : -1;
    int x_dir = x0 < x1 ? 1 : -1;
    int y, x;

    for (y = y0; y_dir == 1 ? y < y1 : y > y1; y += y_dir) {
      if (d->map[y][x0] != -1) {
        d->map[y][x0] = 0;
      }
    }

    for (x = x0; x_dir == 1 ? x < x1 : x > x1; x += x_dir) {
      if (d->map[y1][x] != -1) {
        d->map[y1][x] = 0;
      }
    }

    //add random staircases
    int s = 0, stairs = (rand() % 3) + 2;

    while (s < stairs / 2) {
      int n = (rand() % 20) + 1, m = (rand() % 79) + 1;

      if (d->map[n][m] == 0 || d->map[n][m] == -1) {
        d->map[n][m] = -3; //up
        s++;
      }
    }

    while (s < stairs) {
      int n = (rand() % 20) + 1, m = (rand() % 79) + 1;

      if (d->map[n][m] == 0 || d->map[n][m] == -1) {
        d->map[n][m] = -4; //down
        s++;
      }
    }

    //place pc in room[0] top left
    d->pc.x = d->rooms[0].xpos;
    d->pc.y = d->rooms[0].ypos;
    d->map[d->pc.y][d->pc.x] = -2;
  }

void print_dungeon(dungeon_t *d) {
    int i, j;

    for (i = 0; i < 21; i++) {
      for (j = 0; j < 80; j++) {
        if (d->map[i][j] == 255) {
          printf(" "); //border
        }
        else if (d->map[i][j] == 0) {
          printf("#"); //corridor
        }
        else if (d->map[i][j] == -1) {
          printf("."); //floor
        }
        else if (d->map[i][j] == -2) {
          printf("@"); //player
        }
        else if (d->map[i][j] == -3) {
          printf("<"); //up
        }
        else if (d->map[i][j] == -4) {
          printf(">"); //down
        }
        else {
          printf(" ");
        }
      }
      printf("\n");
    }
  }

int main(int argc, char *argv[]) {
    srand(time(NULL));
    dungeon_t d;
    int swload, swsave;
    swload = swsave = 0;

    char *home = getenv("HOME");
    char *gamedir = ".rlg327";
    char *savefile = "dungeon";
    char *path = malloc(strlen(home) + strlen(gamedir) + strlen(savefile) + 3);

    sprintf(path, "%s/%s/%s", home, gamedir, savefile);


    if (argc > 1) {
      int i;

      for (i = 1; i < argc; i++) {
        if (!strcmp("--load", argv[i])) {
          swload = 1;
        }
        else if (!strcmp("--save", argv[i])) {
          swsave = 1;
        }
      }
    }

    setup(&d);

    if (swload) {   //load
      FILE *f;
      if (!(f = fopen(path, "r"))) {
        fprintf(stderr, "Failed to open file for reading\n");

        return -1;
      }

      char semantic[13];
      semantic[12] = '\0';
      fread(semantic, 1, 12, f);

      uint32_t version;
      fread(&version, 4, 1, f);
      version = be32toh(version);

      uint32_t size;
      fread(&size, 4, 1, f);
      size = be32toh(size);

      fread(&d.pc.x, 1, 1, f);
      fread(&d.pc.y, 1, 1, f);

      int i, j;

      for (i = 0; i < 21; i++) {
        for (j = 0; j < 80; j++) {
          fread(&d.map[i][j], 1, 1, f);
        }
      }

      fread(&d.num_rooms, 2, 1, f);
      d.num_rooms = be16toh(d->num_rooms);
      d.rooms = malloc(d->num_rooms * sizeof(room_t));

      int k;
      for (k = 0; k < d->num_rooms; k++) {
        fread(&d->rooms[k].xpos, 1, 1, f);
        fread(&d->rooms[k].ypos, 1, 1, f);
        fread(&d->rooms[k].xsize, 1, 1, f);
        fread(&d->rooms[k].ysize, 1, 1, f);

        int i, j;

        for (j = d->rooms[k].ypos; j < d->rooms[k].ysize + d->rooms[k].ypos; j++) {
          for (i = d->rooms[k].xpos; i < d->rooms[k].xsize + d->rooms[k].xpos; i++) {
            d->map[j][i] = -1;
          }
        }
      }

      uint16_t upstairs, downstairs;
      fread(&upstairs, 2, 1, f);
      upstairs = be16toh(upstairs);

      int x, y;

      for (i = 0; i < upstairs; i++) {
        fread(&x, 1, 1, f);
        fread(&y, 1, 1, f);
        d->map[y][x] = -3;
      }

      fread(&downstairs, 2, 1, f);
      downstairs = be16toh(downstairs);

      for (i = 0; i < downstairs; i++) {
        fread(&x, 1, 1, f);
        fread(&y, 1, 1, f);
        d->map[y][x] = -4;
      }

      //place pc
      d->map[d->pc.y][d->pc.x] = -2;

      fclose(f);
    }

    else { //dont load - default
      generate_dungeon(&d);
    }

    print_dungeon(&d);

    if (swsave) {
      FILE *f;
      if (!(f = fopen(path, "w"))) {
        fprintf(stderr, "Failed to open file for writing\n");

        return -1;
      }

      char *semantic = "RLG327-S2021";
      int i;
      for (i = 0; i < strlen(semantic); i++) {
        fwrite(&semantic[i], 1, 1, f);
      }

      uint32_t version = 1;
      version = htobe32(version);
      fwrite(&version, 4, 1, f);

      uint32_t size;
      size = htobe32(size);
      fwrite(&size, 4, 1, f);

      //pc
      fwrite(&d->rooms[0].xpos, 1, 1, f);
      fwrite(&d->rooms[0].ypos, 1, 1, f);
    }


    return 0;
  }
