#include <unistd.h>

int create_childs(int num){
  int i;
  for(i=0; i<num; i++){
    switch(fork()){
      case -1:  return -1;
      case 0:   return i+1;
      default:  continue;
    }
  }
  return 0;
}
