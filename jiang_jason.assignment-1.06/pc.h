#ifndef PC_H
# define PC_H

# include <cstdint>

# include "dims.h"
# include "dungeon.h"

class dungeon_t;

class pc_t {
public:
  terrain_type_t remembered[DUNGEON_Y][DUNGEON_X];
};

void pc_delete(pc_t *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
uint32_t pc_in_room(dungeon_t *d, uint32_t room);
void look(dungeon_t *d);
void init_vision(dungeon_t *d);

#endif
