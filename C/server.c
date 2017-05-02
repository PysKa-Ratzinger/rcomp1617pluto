#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include "server.h"
#include "file_storage.h"
#include "init.h"

static char *my_folder;

void end(int signal){
  (void) signal;
  exit(EXIT_SUCCESS);
}

void handle_request(int fd, struct sockaddr_storage *peer_addr,
                    socklen_t peer_addr_len){
  char buffer[FILENAME_SIZE];
  int index = 0, nbyte;

  if(inet_ntop(AF_INET, peer_addr, buffer, peer_addr_len) == NULL)
    handle_error("inet_ntop");

  printf("(%d) Got file request from %s\n", getpid(), buffer);

  while(1){
    nbyte = recv(fd, &buffer[index], 1, 0);
    if(nbyte == 0){
      fprintf(stderr, "(%d) Connection with client terminated "
              "abruptly.\n", getpid());
      exit(EXIT_FAILURE);
    }
    printf("(%d) Received byte '%c'.\n", getpid(), buffer[index]);
    index += nbyte;
  }
}

int start_server(char* folder, int *tcp_port){
  int sock_tcp;
  init_tcp(&sock_tcp);
  init_tcp_port_number(sock_tcp, tcp_port);

  my_folder = folder;

  int pid = fork();
  switch(pid){
    case -1: /* error */
      handle_error("server:fork");

    case 0: /* child behaviour */
      break;

    default: /* parent behaviour */
      close(sock_tcp);
      return pid;
  }

  // SERVER LOOP
  int tcp_connections = 0;
  while(1){
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    int fd;

    fd = accept(sock_tcp, (struct sockaddr*)&peer_addr, &peer_addr_len);
    if(fd == -1){
      if(errno != EINTR){
        handle_error("accept");
      }else{
        continue;
      }
    }

    if(tcp_connections == MAX_TCP_CONN){
      if(waitpid((pid_t)-1, 0, WNOHANG) > 0){
        tcp_connections--;
      }else{
        close(fd);
        printf("Max connections reached. Connection refused.\n");
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
