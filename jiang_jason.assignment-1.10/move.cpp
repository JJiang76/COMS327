#include "move.h"

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "utils.h"
#include "path.h"
#include "event.h"
#include "io.h"
#include "npc.h"
#include "object.h"

void do_combat(dungeon *d, character *atk, character *def)
{
  int32_t damage = 0;
 // bool hit;
  const char *organs[] = {
    "liver",
    "pancreas",
    "heart",
    "brain",
    "eye",
    "arm",
    "leg",
    "intestines",
    "gall bladder",
    "lungs",
    "hand",
    "foot",
    "spinal cord",
    "pituitary gland",
    "thyroid",
    "tongue",
    "bladder",
    "diaphram",
    "frontal lobe",
    "hippocampus",
    "stomach",
    "pharynx",
    "esophagus",
    "trachea",
    "urethra",
    "spleen",
    "cerebellum",
    "ganglia",
    "ear",
    "subcutaneous tissue",
    "prefrontal cortex"
  };
  const char *attacks[] = {
    "punches",
    "kicks",
    "stabs",
    "impales",
    "slashes",
    "massages",
    "soothes",
    "bites",
    "jabs",
    "coerces",
    "threatens",
    "manipulates",
    "arm locks",
    "conquers",
    "buries the hatchet in",
    "indicates displeasure with",
    "quarrels with",
    "scrimmages with",
    "tickles",
    "engages in fisticuffs with",
    "strikes",
    "belts",
    "wallops",
    "gives the old one-two to",
    "bumps into",
    "behaves inappropriately with",
    "smacks",
    "body slams",
    "fondues",
    "flambes",
    "pokes",
    "anoints",
  };
  if (character_is_alive(def)) {
    if (atk != d->PC) {
      if (!(prob(d->PC->calc_dodge()))) {
        damage = atk->damage->roll();
        damage -= d->PC->calc_def();

        if (damage < 0) {
          damage = 0;
        }
        io_queue_message("%s%s %s your %s for %d.", is_unique(atk) ? "" : "The ",
                       atk->name, attacks[rand() % (sizeof (attacks) /
                                                    sizeof (attacks[0]))],
                       organs[rand() % (sizeof (organs) /
                                        sizeof (organs[0]))], damage);
      }
      else {
        io_queue_message("%s%s misses", is_unique(atk) ? "" : "The ", atk->name);
      }
      
    } 
    else {
      if (prob(d->PC->calc_hit())) {
        damage += d->PC->calc_dmg();

        if (prob(d->PC->calc_crit())) {
          damage *= 2;
          io_queue_message("Critical Hit! You hit %s%s for %d.", 
                            is_unique(def) ? "" : "the ", def->name, damage);
        }
        else {
          io_queue_message("You hit %s%s for %d.", is_unique(def) ? "" : "the ",
                        def->name, damage);
        }
      }
      else {
        io_queue_message("You miss %s%s.", is_unique(def) ? "" : "the ", def->name);
      }
      
    }

    if (damage >= (int)def->hp) {
      if (atk != d->PC) {
        io_queue_message("You die.");
        io_queue_message("As %s%s eats your %s,", is_unique(atk) ? "" : "the ",
                         atk->name, organs[rand() % (sizeof (organs) /
                                                     sizeof (organs[0]))]);
        io_queue_message("   ...you wonder if there is an afterlife.");
        /* Queue an empty message, otherwise the game will not pause for *
         * player to see above.                                          */
        io_queue_message("");
      } 
      else {
        io_queue_message("%s%s dies.", is_unique(def) ? "" : "The ", def->name);
        
        int gain = (d->floor_num >= 0 ? d->floor_num + 1 : 1);

        if (d->PC->exp >= d->PC->exp_to_next()) {
          io_queue_message("You must level up before gaining more exp.");
        }
        else {
          if (d->PC->exp + gain >= d->PC->exp_to_next()) {
            gain = d->PC->exp_to_next() - d->PC->exp;
            io_queue_message("You gained %d exp!", gain);
            io_queue_message("You have enough exp to level up! " 
                             "Press \'U\' to level up.");
          }
          else {
            io_queue_message("You gained %d exp!", gain);
          }
          d->PC->exp += gain;
        }

        if (dynamic_cast<npc*>(def)->characteristics & NPC_BOSS) {
          io_queue_message("It seems as if %s dropped a key...", def->name);
          io_queue_message("I wonder what that is for...");
          d->can_rescue = 1;
        }
      }
      def->hp = 0;
      def->alive = 0;
      character_increment_dkills(atk);
      character_increment_ikills(atk, (character_get_dkills(def) +
                                       character_get_ikills(def)));
      if (def != d->PC) {
        d->num_monsters--;
      }
      charpair(def->position) = NULL;
    } 
    else {
      def->hp -= damage;
    }
  }
}

void move_character(dungeon *d, character *c, pair_t next)
{
  int can_see_atk, can_see_def;
  pair_t displacement;
  uint32_t found_cell;
  pair_t order[9] = {
    { -1, -1 },
    { -1,  0 },
    { -1,  1 },
    {  0, -1 },
    {  0,  0 },
    {  0,  1 },
    {  1, -1 },
    {  1,  0 },
    {  1,  1 },
  };
  uint32_t s, i;

  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x]))) {
    if ((charpair(next) == d->PC) ||
        c == d->PC) {
      do_combat(d, c, charpair(next));
    } else {
      /* Easiest way for a monster to displace another monster is *
       * to swap them.  This could lead to some strangeness where *
       * two monsters of the exact same speed continually         *
       * displace each other and never make progress, but I don't *
       * have any real problem with that.  When we have better    *
       * game balance, weaker monsters should not be able to      *
       * displace stronger monsters.                              */
      /* Turns out I don't like swapping them after all.  We'll   *
       * instead select a random square from the 8 surrounding    *
       * the target cell.  Keep doing it until either we swap or  *
       * find an empty one for the displacement.                  */
      for (s = rand() % 9, found_cell = i = 0;
           i < 9 && !found_cell; i++) {
        displacement[dim_y] = next[dim_y] + order[s % 9][dim_y];
        displacement[dim_x] = next[dim_x] + order[s % 9][dim_x];
        if (((npc *) charpair(next))->characteristics & NPC_PASS_WALL) {
          if (!charpair(displacement) ||
              (charpair(displacement) == c)) {
            found_cell = 1;
          }
        } else {
          if ((!charpair(displacement) &&
               (mappair(displacement) >= ter_floor)) ||
              (charpair(displacement) == c)) {
            found_cell = 1;
          }
        }
      }

      if (!found_cell) {
        return;
      }

      assert(charpair(next));

      can_see_atk = can_see(d, character_get_pos(d->PC),
                            character_get_pos(c), 1, 0);
      can_see_def = can_see(d, character_get_pos(d->PC),
                            character_get_pos(charpair(next)), 1, 0);

      if (can_see_atk && can_see_def) {
        io_queue_message("%s%s pushes %s%s out of the way.  How rude.",
                         is_unique(c) ? "" : "The ", c->name,
                         is_unique(charpair(next)) ? "" : "the ",
                         charpair(next)->name);
      } else if (can_see_atk) {
        io_queue_message("%s%s angrily shoves something out of the way.",
                         is_unique(c) ? "" : "The ", c->name);
      } else if (can_see_def) {
        io_queue_message("Something slams %s%s out of the way.",
                          is_unique(charpair(next)) ? "" : "the ",
                         charpair(next)->name);
      }

      charpair(c->position) = NULL;
      charpair(displacement) = charpair(next);
      charpair(next) = c;
      charpair(displacement)->position[dim_y] = displacement[dim_y];
      charpair(displacement)->position[dim_x] = displacement[dim_x];
      c->position[dim_y] = next[dim_y];
      c->position[dim_x] = next[dim_x];
    }
  } else {
    /* No character in new position. */

    d->character_map[c->position[dim_y]][c->position[dim_x]] = NULL;
    c->position[dim_y] = next[dim_y];
    c->position[dim_x] = next[dim_x];
    d->character_map[c->position[dim_y]][c->position[dim_x]] = c;
  }

  if (c == d->PC) {
    pc_reset_visibility((pc *) c);
    if (d->floor_num != 10) {
      pc_observe_terrain((pc *) c, d);
    }
  }
}

void do_moves(dungeon *d)
{
  pair_t next;
  character *c;
  event *e;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    /* The PC always goes first one a tie, so we don't use new_event().  *
     * We generate one manually so that we can set the PC sequence       *
     * number to zero.                                                   */
    e = (event *) malloc(sizeof (*e));
    e->type = event_character_turn;
    /* Hack: New dungeons are marked.  Unmark and ensure PC goes at d->time, *
     * otherwise, monsters get a turn before the PC.                         */
    if (d->is_new) {
      d->is_new = 0;
      e->time = d->time;
    } else {
      e->time = d->time + (1000 / d->PC->speed);
    }
    e->sequence = 0;
    e->c = d->PC;
    heap_insert(&d->events, e);
  }

  while (pc_is_alive(d) &&
         (e = (event *) heap_remove_min(&d->events)) &&
         ((e->type != event_character_turn) || (e->c != d->PC))) {
    d->time = e->time;
    if (e->type == event_character_turn) {
      c = e->c;
    }
    if (!c->alive) {
      if (d->character_map[c->position[dim_y]][c->position[dim_x]] == c) {
        d->character_map[c->position[dim_y]][c->position[dim_x]] = NULL;
      }
      if (c != d->PC) {
        event_delete(e);
      }
      continue;
    }

    npc_next_pos(d, (npc *) c, next);
    move_character(d, (npc *) c, next);

    heap_insert(&d->events, update_event(d, e, 1000 / c->speed));
  }

  io_display(d);
  if (pc_is_alive(d) && e->c == d->PC) {
    c = e->c;
    d->time = e->time;
    /* Kind of kludgey, but because the PC is never in the queue when   *
     * we are outside of this function, the PC event has to get deleted *
     * and recreated every time we leave and re-enter this function.    */
    e->c = NULL;
    event_delete(e);
    io_handle_input(d);
  }
}

void dir_nearest_wall(dungeon *d, character *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (c->position[dim_x] != 1 && c->position[dim_x] != DUNGEON_X - 2) {
    dir[dim_x] = (c->position[dim_x] > DUNGEON_X - c->position[dim_x] ? 1 : -1);
  }
  if (c->position[dim_y] != 1 && c->position[dim_y] != DUNGEON_Y - 2) {
    dir[dim_y] = (c->position[dim_y] > DUNGEON_Y - c->position[dim_y] ? 1 : -1);
  }
}

uint32_t against_wall(dungeon *d, character *c)
{
  return ((mapxy(c->position[dim_x] - 1,
                 c->position[dim_y]    ) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x] + 1,
                 c->position[dim_y]    ) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x]    ,
                 c->position[dim_y] - 1) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x]    ,
                 c->position[dim_y] + 1) == ter_wall_immutable));
}

uint32_t in_corner(dungeon *d, character *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(c->position[dim_x] - 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x] + 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] - 1) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

static void new_dungeon_level(dungeon *d, uint32_t dir)
{
  /* Eventually up and down will be independantly meaningful. *
   * For now, simply generate a new dungeon.                  */

  switch (dir) {
  case '<':
    io_queue_message("You go up the stairs.");
    io_queue_message(""); /* To force "more" */
    io_display(d); /* To force queue flush */
    if (d->floor_num != 10) {
      d->floor_num++;
    }
    new_dungeon(d);
    break;
  case '>':
    io_queue_message("You go down the stairs.");
    io_queue_message(""); /* To force "more" */
    io_display(d); /* To force queue flush */
    d->floor_num--;
    new_dungeon(d);
    break;
  default:
    break;
  }
}


uint32_t move_pc(dungeon *d, uint32_t dir)
{
  pair_t next;
  uint32_t was_stairs = 0;
  const char *wallmsg[] = {
    "There's a wall in the way.",
    "BUMP!",
    "Ouch!",
    "You stub your toe.",
    "You can't go that way.",
    "You admire the engravings.",
    "Are you drunk?"
  };

  next[dim_y] = d->PC->position[dim_y];
  next[dim_x] = d->PC->position[dim_x];


  switch (dir) {
  case 1:
  case 2:
  case 3:
    next[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    next[dim_y]--;
    break;
  }
  switch (dir) {
  case 1:
  case 4:
  case 7:
    next[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    next[dim_x]++;
    break;
  case '<':
    if (mappair(d->PC->position) == ter_stairs_up) {
      was_stairs = 1;
      new_dungeon_level(d, '<');
    }
    break;
  case '>':
    if (mappair(d->PC->position) == ter_stairs_down) {
      was_stairs = 1;
      new_dungeon_level(d, '>');
    }
    break;
  }

  if (was_stairs) {
    return 0;
  }

  if ((dir != '>') && (dir != '<') && (mappair(next) >= ter_floor)) {
    move_character(d, d->PC, next);
    dijkstra(d);
    dijkstra_tunnel(d);
    d->PC->pick_up(d);

    return 0;
  } 
  else if (mappair(next) < ter_floor) {
    if (mappair(next) == ter_cage_door) {
      io_queue_message("You have found Princess Brook!!");
      if (d->can_rescue) {
        io_queue_message("You try the key that you risked your life to get");
        io_queue_message("to open the door.");
        io_queue_message("Success! The key works!");
        mapxy(40, 6) = ter_floor_room;
      }
      else {
        io_queue_message("It seems as if her prison is locked tight.");
        io_queue_message("No matter how hard you try, the lock won't budge.");
        io_queue_message("It looks like you have to find "
                          "the owner of the key...");
      }
    }
    else if (mappair(next) == ter_princess) {
      d->quit = 1;
      return 0;
    }
    else {
      io_queue_message(wallmsg[rand() % (sizeof (wallmsg) /
                                     sizeof (wallmsg[0]))]);  
    }
    io_display(d);
  }

  return 1;
}
