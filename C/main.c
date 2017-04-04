#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "init.h"

int main(){
  struct main_var vars;
  ssize_t nbyte;
  int i;
  char buffer[BUFFER_SIZE];

  init(&vars);

  for(i=0; i<10; i++){
    snprintf(buffer, BUFFER_SIZE, "%d: %d\n", i, vars.tcp_port);
    nbyte = sendto(vars.sock_udp, buffer, strlen(buffer), 0,
                    (struct sockaddr*)&vars.bcast_addr, vars.bcast_addrlen);
    if(nbyte == -1){
      perror("sendto");
      exit(EXIT_FAILURE);
    }

    sleep(1);
  }

  return 0;
}

