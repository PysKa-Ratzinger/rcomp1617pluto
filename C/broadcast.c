#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "broadcast.h"

#define BUFFER_SIZE 512

void start_broadcast(struct main_var *vars){
  int nbyte;
  char buffer[BUFFER_SIZE];

  while(1){
    snprintf(buffer, BUFFER_SIZE, "tport:%dd%se", vars->tcp_port, "hello :D");
    nbyte = sendto(vars->sock_udp, buffer, strlen(buffer), 0,
                    (struct sockaddr*)&vars->bcast_addr, vars->bcast_addrlen);
    if(nbyte == -1){
      perror("sendto");
      exit(EXIT_FAILURE);
    }
    sleep(1);
  }
}
