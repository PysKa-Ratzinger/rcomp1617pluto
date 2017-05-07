#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "utils.h"

int num_places(unsigned int n){
  if (n < 10) return 1;
  if (n < 100) return 2;
  if (n < 1000) return 3;
  if (n < 10000) return 4;
  if (n < 100000) return 5;
  if (n < 1000000) return 6;
  if (n < 10000000) return 7;
  if (n < 100000000) return 8;
  if (n < 1000000000) return 9;
  /*      4294967296 is 2^32 -1, so... */
  return 10;
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

size_t remove_newline(char* string){
  return remove_n_newline(string, strlen(string));
}

size_t remove_n_newline(char* string, size_t string_size){
  if(string_size == 0) return 0;
  char *p = string + string_size - 1;
  if(*p == '\n'){
    *p = '\0';
    return string_size - 1;
  }
  return string_size;
}
