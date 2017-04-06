#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "utils.h"

int num_places(unsigned short n){
  if (n < 10) return 1;
  if (n < 100) return 2;
  if (n < 1000) return 3;
  if (n < 10000) return 4;
  /*      65535 is 2^16 -1, so... */
  return 5;
}

void read_folder(char* folder, size_t buffer_size){
  struct stat sb;

  printf("Which folder would you like to share over the network?\n>");
  while(1){
    fflush(stdout);
    fgets(folder, buffer_size, stdin);
    folder[strlen(folder)-1] = 0;

    if(stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode))
      break;  // Successfully read a folder

    printf("Please insert a valid path for a folder. "
            "Has to be a full path.\n>");
  }
}
