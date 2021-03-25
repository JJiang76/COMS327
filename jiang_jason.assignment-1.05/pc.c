#include <stdlib.h>
#include <ncurses.h>

#include "string.h"

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"

void pc_delete(pc_t *pc)
{
  if (pc) {
    free(pc);
  }
}

uint32_t pc_is_alive(dungeon_t *d)
{
  return d->pc.alive;
}

void place_pc(dungeon_t *d)
{
  d->pc.position[dim_y] = rand_range(d->rooms->position[dim_y],
                                     (d->rooms->position[dim_y] +
                                      d->rooms->size[dim_y] - 1));
  d->pc.position[dim_x] = rand_range(d->rooms->position[dim_x],
                                     (d->rooms->position[dim_x] +
                                      d->rooms->size[dim_x] - 1));
}

void config_pc(dungeon_t *d)
{
  memset(&d->pc, 0, sizeof (d->pc));
  d->pc.symbol = '@';

  place_pc(d);

  d->pc.speed = PC_SPEED;
  d->pc.alive = 1;
  d->pc.sequence_number = 0;
  d->pc.pc = calloc(1, sizeof (*d->pc.pc));
  d->pc.npc = NULL;
  d->pc.kills[kill_direct] = d->pc.kills[kill_avenged] = 0;

  d->character[d->pc.position[dim_y]][d->pc.position[dim_x]] = &d->pc;

  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t hit_wall(dungeon_t *d, pair_t dir) {
  return mapxy(d->pc.position[dim_x] + dir[dim_x],
    d->pc.position[dim_y] + dir[dim_y]) == ter_wall ||
    mapxy(d->pc.position[dim_x] + dir[dim_x],
      d->pc.position[dim_y] + dir[dim_y]) == ter_wall_immutable;
}

uint32_t on_upstairs(dungeon_t *d) {
  return mapxy(d->pc.position[dim_x], d->pc.position[dim_y]) == ter_stairs_up;
}

uint32_t on_downstairs(dungeon_t *d) {
  return mapxy(d->pc.position[dim_x], d->pc.position[dim_y]) == ter_stairs_down;
}

void print_list(dungeon_t *d, WINDOW *win, character_t **list, int size, int start) {
  int i;
  int x = 3;
  int y = start;

  for (i = 0; i < size; i++) {
    mvwprintw(win, y, x, "%s is in position (%d, %d).",
      list[i]->symbol, list[i]->position[dim_y], list[i]->position[dim_x]);
    y++;
    wrefresh(win);
  }
}

void display_monsters(dungeon_t *d) {
  uint32_t displaying = 1, i, j, k = 0;
  int32_t key;

  character_t **char_list = malloc(sizeof(character_t) * d->max_monsters);

  for (i = 0; i < DUNGEON_Y; i++) {
    for (j = 0; j< DUNGEON_X; j++) {
      if (d->character[i][j] && d->character[i][j]->npc != NULL) {
        char_list[k] = d->character[i][j];
        k++;
      }
    }
  }

  WINDOW *monster_win = newwin(17, 40, 2, 20);
  box(monster_win, 0, 0);

  mvwprintw(monster_win, 1, 8, "There are %d monsters", k);

//  print_list(d, monster_win, char_list, k - 1, 2);

  wrefresh(monster_win);

  keypad(stdscr, FALSE);

  while (displaying) {
    key = getch();

    switch(key) {
      case 27:
        displaying = 0;
        break;
    }
  }
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  uint32_t moved = 0, key, no_op;
  dir[dim_y] = dir[dim_x] = 0;

  while(!moved) {
    no_op = 0;
    key = getch();

    switch(key) {
      case 7: case 'y':
        dir[dim_y] = -1; dir[dim_x] = -1; break;
      case 8: case 'k':
        dir[dim_y] = -1; break;
      case 9: case 'u':
        dir[dim_y] = -1; dir[dim_x] = 1; break;
      case 6: case 'l':
        dir[dim_x] = 1; break;
      case 3: case 'n':
        dir[dim_y] = 1; dir[dim_x] = 1; break;
      case 2: case 'j':
        dir[dim_y] = 1; break;
      case 1: case 'b':
        dir[dim_y] = 1; dir[dim_x] = -1; break;
      case 4: case 'h':
        dir[dim_x] = -1; break;
      case 5: case ' ': case '.':
        dir[dim_y] = dir[dim_x] = 0; break;
      case '>':
        if (on_downstairs(d)) {
          mvprintw(0, 1, "Going down!");
          refresh();
          dir[dim_y] = dir[dim_x] = 0;
        }
        else {
          no_op = 1;
        }
        break;
      case '<':
      if (on_upstairs(d)) {
        mvprintw(0, 1, "Going up!");
        refresh();
        dir[dim_y] = dir[dim_x] = 0;
      }
      else {
        no_op = 1;
      }
      break;
      case 'm':
        display_monsters(d);
        render_dungeon(d);
        no_op = 1;
        break;
      case 'Q':
        d->pc.alive = 0; break;
      default:
        mvprintw(0, 1, "Use directional buttons to move!");
        refresh();
        no_op = 1;
    }
    if (!hit_wall(d, dir) && !no_op) {
        moved = 1;
    }
    else {
      mvprintw(0, 1, "Try again - make a smarter move");
      refresh();
      dir[dim_y] = dir[dim_x] = 0;
    }
  }

  return 0;
}

uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
  if ((room < d->num_rooms)                                     &&
      (d->pc.position[dim_x] >= d->rooms[room].position[dim_x]) &&
      (d->pc.position[dim_x] < (d->rooms[room].position[dim_x] +
                                d->rooms[room].size[dim_x]))    &&
      (d->pc.position[dim_y] >= d->rooms[room].position[dim_y]) &&
      (d->pc.position[dim_y] < (d->rooms[room].position[dim_y] +
                                d->rooms[room].size[dim_y]))) {
    return 1;
  }

  return 0;
}
