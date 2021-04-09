#ifndef OBJECTS_H
# define OBJECTS_H

# include <stdint.h>
# include <string>
# include <vector>

# include "dice.h"

typedef struct dungeon dungeon_t;

typedef enum object_type {
  objtype_no_type,
  objtype_WEAPON,
  objtype_OFFHAND,
  objtype_RANGED,
  objtype_LIGHT,
  objtype_ARMOR,
  objtype_HELMET,
  objtype_CLOAK,
  objtype_GLOVES,
  objtype_BOOTS,
  objtype_AMULET,
  objtype_RING,
  objtype_SCROLL,
  objtype_BOOK,
  objtype_FLASK,
  objtype_GOLD,
  objtype_AMMUNITION,
  objtype_FOOD,
  objtype_WAND,
  objtype_CONTAINER
} object_type_t;

extern const char object_symbol[];

class object_t {
public:
  std::string name, description;
  object_type_t type;
  uint32_t color;
  dice damage;
  int hit, dodge, defence, weight, speed, attribute, value;
  bool artifact;
  uint32_t rarity;

  object_t() : name(),    description(),            type(objtype_no_type),
                          color(0),  damage(),         hit(0),
                          dodge(0),   defence(0),     weight(0),
                          speed(0),   attribute(0),   value(0),
                          artifact(false), rarity(0) {}

  object_t(std::string name, std::string description, object_type_t type,
           uint32_t color, dice damage, int hit, int dodge, int defence,
           int weight, int speed, int attribute, int value, bool artifact,
           uint32_t rarity) :

           name(name), description(description), type(type),
           color(color), damage(damage), hit(hit), dodge(dodge),
           defence(defence), weight(weight), speed(speed), attribute(attribute),
           value(value), artifact(artifact), rarity(rarity) {}
};

void gen_objects(dungeon_t *d);

#endif
