#ifndef PARSE_H
#define PARSE_H

#include <string>
#include <vector>

#include "utils.h"

class monster_desc {
public:
  std::string name;
  std::string desc;
  char symb;
  std::vector<int> color;
  int abils;
  dice speed;
  dice hp;
  dice ad;
  int rarity;

  monster_desc() : name(), desc(), symb(0),   color(0),
                          abils(0), speed(), hp(), ad(), rarity(0)
  {
  }
};

void parse(std::vector<monster_desc> &v);
void print_desc(std::vector<monster_desc> &v);


#endif
