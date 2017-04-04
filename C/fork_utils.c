#include "fork_utils.h"

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

int psystem(char *cmd){
  int fd[2];

  if(pipe(fd) == -1){
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  switch(create_childs(1)){
    case -1:
      perror("create_childs");
      exit(EXIT_FAILURE);

    case 0: /* parent process */
      close(fd[1]);
      return fd[0];

    case 1: /* child process */
      dup2(fd[1], 1);
      close(fd[0]);
      close(fd[1]);
      execl("/bin/sh", "sh", "-c", cmd, (char *)0);
      perror("execl");
      exit(EXIT_FAILURE);
  }
}
