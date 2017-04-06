#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "init.h"
#include "fork_utils.h"
#include "file_storage.h"
#include "broadcast.h"
#include "utils.h"

#define BUFFER_SIZE 256

static struct main_var vars;

void ctrl_c_handler(int signal){
  (void)signal;
  printf("\nInterrupt detected... cleaning up.\n");
  exit(EXIT_SUCCESS);
}

void cleanup(){
  close(vars.sock_udp);
  close(vars.sock_ft);
  freefsinfo(vars.storage);
}

int main(){
  char folder[BUFFER_SIZE];

  if(signal(SIGINT, ctrl_c_handler) == SIG_ERR){
    fprintf(stderr, "Could not set-up interrupt handler.\n");
    exit(EXIT_FAILURE);
  }

  if(atexit(cleanup) != 0){
    perror("atexit");
    exit(EXIT_FAILURE);
  }

  init(&vars);
  read_folder(folder, BUFFER_SIZE);
  start_broadcast(&vars, folder);

  printf("Press any key when you want the child to stop broadcasting.\n");
  getchar();
  stop_broadcast();

  return 0;
}
