#include "dungeon.h"
#include "objects.h"
#include "utils.h"

#define NUM_OBJ 10


void gen_objects(dungeon_t *d) {
  int i, j;
  uint32_t k;
  object_t *o;
  object_description desc;
  uint32_t room;
  pair_t p;

  d->num_objs = NUM_OBJ;

  for (i = 0; i < d->num_objs; i++) {
    int generating = 1;
    while (generating) {
      j = rand_range(0, d->object_descriptions.size() - 1);
      desc = d->object_descriptions[j];
      k = rand_range(0, 99);

      if (desc.get_artifact()) {
        if (d->artifact_obtained || k >= desc.get_rarity()) {
          continue;
        }
        d->artifact_obtained = true;
      }

      o = desc.generate_object();
      generating = 0;
    }

    do {
      room = rand_range(1, d->num_rooms - 1);
      p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                            (d->rooms[room].position[dim_y] +
                             d->rooms[room].size[dim_y] - 1));
      p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                            (d->rooms[room].position[dim_x] +
                             d->rooms[room].size[dim_x] - 1));
    } while (d->object_map[p[dim_y]][p[dim_x]]);

    d->object_map[p[dim_y]][p[dim_x]] = o;
  }
}
