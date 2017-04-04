#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

#include "init.h"
#include "fork_utils.h"

#define BUFFER_SIZE 256

static struct main_var vars;

void ctrl_c_handler(int signal){
  printf("\nInterrupt detected... cleaning up.\n");
  close(vars.sock_udp);
  close(vars.sock_ft);
  exit(EXIT_SUCCESS);
}

int main(){
  struct stat sb;
  ssize_t nbyte;
  int i, err, fd;
  char buffer[BUFFER_SIZE];
  char folder[BUFFER_SIZE];

  if(signal(SIGINT, ctrl_c_handler) == SIG_ERR){
    fprintf(stderr, "Could not set-up interrupt handler.\n");
    exit(EXIT_FAILURE);
  }

  init(&vars);

  err = 1;
  printf("Which folder would you like to share over the network?\n>");
  while(err){
    fflush(stdout);
    fgets(folder, BUFFER_SIZE, stdin);
    folder[strlen(folder)-1] = 0;

    if(stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode))
      break;  // Successfully read a folder

    printf("Please insert a valid path for a folder."
            "Has to be a full path.\n>");
  }

  snprintf(buffer, BUFFER_SIZE, "ls -p %s | grep -v /", folder);
  system(buffer);
  printf("Broadcasting folder \"%s\" over the network... ", folder);
  fflush(stdout);

  for(i=0; i<10; i++){
    snprintf(buffer, BUFFER_SIZE, "tport:%dd%se", vars.tcp_port, folder);
    nbyte = sendto(vars.sock_udp, buffer, strlen(buffer), 0,
                    (struct sockaddr*)&vars.bcast_addr, vars.bcast_addrlen);
    if(nbyte == -1){
      perror("sendto");
      exit(EXIT_FAILURE);
    }

    sleep(1);
  }

  printf("done.\n");

  return 0;
}
