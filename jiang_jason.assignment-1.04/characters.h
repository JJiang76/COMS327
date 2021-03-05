#ifndef CHARACTERS_H
#define CHARACTERS_H

#define BIT_SMART 0x1
#define BIT_TELE 0x2
#define BIT_TUNNEL 0x4
#define BIT_ERR = 0x8

#define DEFAULT_MONS 8;

typedef struct pc {
  pair_t position;
} pc_t;

typedef struct npc {
  int characteristics;
} npc_t;

typedef struct character {
  pc_t *pc;
  npc_t *npc;
  int x_pos, y_pos;
  int next_turn;
  int speed;
  int sequence;
  int is_alive;
} character_t;

#endif
