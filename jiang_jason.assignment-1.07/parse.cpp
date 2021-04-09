#include <fstream>
#include <iostream>
#include <cstring>
#include <ncurses.h>

#include "parse.h"
#include "npc.h"

using namespace std;

void parse(vector<monster_desc>& v) {
  char *home = getenv("HOME");
  char *gamedir = (char *) ".rlg327";
  char *loadfile = (char *) "monster_desc.txt";
  char *path = (char *) malloc(strlen(home) + strlen(gamedir) + strlen(loadfile) + 3);
  sprintf(path, "%s/%s/%s", home, gamedir, loadfile);
  ifstream f(path);
  string s;
  int count;

  getline(f, s);

  if (s.compare("RLG327 MONSTER DESCRIPTION 1") != 0) {
    cout << "Invalid file name. Exiting..." << endl;
  }

  while (f.peek() != EOF) {
    while (f.peek() != 'B' && f.peek() != EOF) {
      f.get();
    }
    getline(f, s);
    if (s.compare("BEGIN MONSTER") == 0) {
      monster_desc m;
      count = 0;
      while (s != "END") {
        f >> s;

        if (s.compare("NAME") == 0) {
          f.get();
          getline(f, s);
          m.name = s;
          count++;
        }
        else if (s.compare("SYMB") == 0) {
          f.get();
          m.symb = f.get();
          count++;
        }
        else if (s.compare("COLOR") == 0) {
          while (f.peek() != '\n') {
            f >> s;

            if (s.compare("RED") == 0) {
              m.color.push_back(COLOR_RED);
            }
            else if (s.compare("GREEN") == 0) {
              m.color.push_back(COLOR_GREEN);
            }
            else if (s.compare("BLUE") == 0) {
              m.color.push_back(COLOR_BLUE);
            }
            else if (s.compare("CYAN") == 0) {
              m.color.push_back(COLOR_CYAN);
            }
            else if (s.compare("YELLOW") == 0) {
              m.color.push_back(COLOR_YELLOW);
            }
            else if (s.compare("MAGENTA") == 0) {
              m.color.push_back(COLOR_MAGENTA);
            }
            else if (s.compare("WHITE") == 0) {
              m.color.push_back(COLOR_WHITE);
            }
            else if (s.compare("BLACK") == 0) {
              m.color.push_back(COLOR_BLACK);
            }
          }

          count++;
        }
        else if (s.compare("DESC") == 0) {
          while (f.peek() != EOF) {
            getline(f, s);
            if (s.length() > 77) {
              break;
            }

            s.append("\n");

            if (s == ".\n") {
              break;
            }

            m.desc += s;
          }
          count++;
        }
        else if (s.compare("SPEED") == 0) {
          int base, num_dice, sides;
          f >> s;

          sscanf(s.c_str(), "%d+%ud%u", &base, &num_dice, &sides);

          m.speed.set(base, num_dice, sides);

          count++;
        }
        else if (s.compare("DAM") == 0) {
          int base, num_dice, sides;
          f >> s;

          sscanf(s.c_str(), "%d+%ud%u", &base, &num_dice, &sides);

          m.ad.set(base, num_dice, sides);

          count++;
        }
        else if (s.compare("HP") == 0) {
          int base, num_dice, sides;
          f >> s;

          sscanf(s.c_str(), "%d+%ud%u", &base, &num_dice, &sides);

          m.hp.set(base, num_dice, sides);

          count++;
        }
        else if (s.compare("RRTY") == 0) {
          f >> s;

          sscanf(s.c_str(), "%d", &m.rarity);
          count++;
        }
        else if (s.compare("ABIL") == 0) {
          int abils = 0;

          while (f.peek() != '\n') {
            f >> s;

            if (s == "SMART") {
              abils |= NPC_SMART;
            }
            else if (s == "TELE") {
              abils |= NPC_TELEPATH;
            }
            else if (s == "TUNNEL") {
              abils |= NPC_TUNNEL;
            }
            else if (s == "ERRATIC") {
              abils |= NPC_ERRATIC;
            }
            else if (s == "PASS") {
              abils |= NPC_PASS_WALL;
            }
            else if (s == "PICKUP") {
              abils |= NPC_PICKUP_OBJ;
            }
            else if (s == "DESTROY") {
              abils |= NPC_DESTROY_OBJ;
            }
            else if (s == "UNIQ") {
              abils |= NPC_UNIQ;
            }
            else if (s == "BOSS") {
              abils |= NPC_BOSS;
            }
          }
          m.abils = abils;
          count++;
        }
        else if (s.compare("END") == 0) {
          if (count != 9) {
            cout << "bruh" << endl;
            break;
          }
        }
      }
      if (count != 9) {
        cout << "we got a problem" << endl;
      }
      else {
        (&v)->push_back(m);
      }
    }
  }
}

void print_desc(vector<monster_desc> &v) {
  int i, j;

  for (i = 0; i < (int) v.size(); i++) {
    cout << v[i].name << v[i].desc;
    cout << v[i].symb << endl;

    for (j = 0; j < (int)v[i].color.size(); j++) {
      if (v[i].color[j] == COLOR_RED) {
        cout << "RED ";
      }
      else if (v[i].color[j] == COLOR_GREEN) {
        cout << "GREEN ";
      }
      else if (v[i].color[j] == COLOR_BLUE) {
        cout << "BLUE ";
      }
      else if (v[i].color[j] == COLOR_CYAN) {
        cout << "CYAN ";
      }
      else if (v[i].color[j] == COLOR_YELLOW) {
        cout << "YELLOW ";
      }
      else if (v[i].color[j] == COLOR_MAGENTA) {
        cout << "MAGENTA ";
      }
      else if (v[i].color[j] == COLOR_WHITE) {
        cout << "WHITE ";
      }
      else if (v[i].color[j] == COLOR_BLACK) {
        cout << "BLACK ";
      }
    }
    cout << endl;
    cout << v[i].abils << endl;

    if (v[i].abils & NPC_SMART) {
      cout << "SMART ";
    }
    if (v[i].abils & NPC_TELEPATH) {
      cout << "TELE ";
    }
    if (v[i].abils & NPC_TUNNEL) {
      cout << "TUNNEL ";
    }
    if (v[i].abils & NPC_ERRATIC) {
      cout << "ERRATIC ";
    }
    if (v[i].abils & NPC_PASS_WALL) {
      cout << "PASS ";
    }
    if (v[i].abils & NPC_PICKUP_OBJ) {
      cout << "PICKUP ";
    }
    if (v[i].abils & NPC_DESTROY_OBJ) {
      cout << "DESTROY ";
    }
    if (v[i].abils & NPC_UNIQ) {
      cout << "UNIQ ";
    }
    if (v[i].abils & NPC_BOSS) {
      cout << "BOSS ";
    }

    cout << endl;
    printf("%d+%dd%d\n", v[i].speed.base, v[i].speed.num_dice, v[i].speed.sides);
    printf("%d+%dd%d\n", v[i].hp.base, v[i].hp.num_dice, v[i].hp.sides);
    printf("%d+%dd%d\n", v[i].ad.base, v[i].ad.num_dice, v[i].ad.sides);
    cout << v[i].rarity << endl << endl;
  }
}
