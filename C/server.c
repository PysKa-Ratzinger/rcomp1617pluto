#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/wait.h>

#include "server.h"
#include "init.h"

static char *my_folder;

void end(int signal){
  (void) signal;
  exit(EXIT_SUCCESS);
}

void handle_request(int fd, struct sockaddr_storage *peer_addr,
                    socklen_t peer_addr_len){
  // char buffer[4096];
  // char folder[1024];
  (void) fd;
  (void) peer_addr;
  (void) peer_addr_len;

  // TODO: Finish function
}

int start_server(char* folder, int *tcp_port){
  int sock_tcp;
  init_tcp(&sock_tcp);
  init_tcp_port_number(sock_tcp, tcp_port);

  my_folder = folder;

  int pid;
  switch((pid = fork())){
    case -1: /* error */
      handle_error("server:fork");

    case 0: /* child behaviour */
      break;

    default: /* parnet behaviour */
      close(sock_tcp);
      return pid;
  }

  // SERVER LOOP
  int tcp_connections = 0;
  while(1){
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    int fd;

    if(tcp_connections == MAX_TCP_CONN){
      wait(0);
      tcp_connections--;
    }

    fd = accept(sock_tcp, (struct sockaddr*)&peer_addr, &peer_addr_len);
    if(fd == -1){
      if(errno != EINTR){
        handle_error("accept");
      }else{
        continue;
      }
    }

    tcp_connections++;
    switch(fork()){
      case -1:
        perror("fork");
        fprintf(stderr, "server: Could not respond to connection.\n");
        break;

      case 0: /* Child behaviour */
        handle_request(fd, &peer_addr, peer_addr_len);
        break;

      default:  /* Parent behaviour */
        close(fd); // We don't need the file descriptor anymore
        break;
    }
  }

  exit(EXIT_SUCCESS);
}
