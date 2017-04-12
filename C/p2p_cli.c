#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#include "init.h"
#include "p2p_cli.h"
#include "broadcast.h"

#define BUFFER_SIZE 512

static sem_t* sem = NULL;

void cleanup_cli(){
  if(sem) sem_close(sem);
}

void start_cli(struct control_st *ctrl){
  int err;
  char buffer[BUFFER_SIZE];
  struct in_addr bf_addr;

  if(atexit(cleanup_cli) != 0){
    perror("atexit");
    exit(EXIT_FAILURE);
  }

  if((sem = sem_open(SEM_OP_NAME, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  printf(">");
  fflush(stdout);
  while(fgets(buffer, BUFFER_SIZE, stdin)){
    char *p = buffer;
    char *end = buffer + BUFFER_SIZE;

    while(p != end){
      if(*p == '\n'){
        *p = 0;
        break;
      }
      p++;
    }

    if(strcmp(buffer, "stop") == 0){
      printf("Stopping the program.\n");
      stop_broadcast(ctrl->broadcast_pid);
      return;
    }else if(strcmp(buffer, "help") == 0){
      printf("List of commands:\n");
      printf("\tstop - stops the program completely\n");
      printf("\tlist - lists the IP of every peer connected\n");
      printf("\tlist IP - if a peer is connected, lists the files\n");
      printf("\t          shared by said peer\n");
      printf("\tget IP FILE LOCATION - if IP is the ip of a conected peer\n");
      printf("\t                       and FILE exists in the list of\n");
      printf("\t                       shared files of that peer, attempts\n");
      printf("\t                       to download FILE into LOCATION\n");
    }else if(strncmp(buffer, "list", 4) == 0){
      if(buffer[4] == '\0'){
        write(ctrl->udp_recv_pipe, "list\0", 5);
        kill(ctrl->udp_recv_pid, SIGUSR1);
        sem_wait(sem);  // Wait for process to be over
      }else{
        err = inet_pton(AF_INET, buffer+5, &bf_addr);
        if(err != 1){
          // TODO: list ip
        }
      }
    }else if(strncmp(buffer, "get ", 4) == 0){
      err = inet_pton(AF_INET, buffer+5, &bf_addr);
      if(err != 1){
        // TODO: get IP FILE LOCATION
      }
    }
    printf(">");
    fflush(stdout);
  }
}
