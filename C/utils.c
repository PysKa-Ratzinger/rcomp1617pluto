#include "utils.h"

int num_places(unsigned short n){
  if (n < 10) return 1;
  if (n < 100) return 2;
  if (n < 1000) return 3;
  if (n < 10000) return 4;
  /*      65535 is 2^16 -1, so... */
  return 5;
}
