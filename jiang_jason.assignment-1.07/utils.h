#ifndef UTILS_H
# define UTILS_H

# include <assert.h>
# include <stdlib.h>

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

#define malloc(size) ({          \
  void *_tmp;                    \
  assert((_tmp = malloc(size))); \
  _tmp;                          \
})

#define UNUSED(f) ((void) f)

int makedirectory(char *dir);

class dice {
public:
  int base, num_dice, sides;

  dice() : base(0), num_dice(0), sides(0) {}

  dice(int base, int num_dice, int sides) :
  base(base), num_dice(num_dice), sides(sides) {}

  void set(int base, int num_dice, int sides) {
    this->base = base;
    this->num_dice = num_dice;
    this->sides = sides;
  }
};

#endif
