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
    struct in_addr bf_addr;

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
      return;
    }else if(strcmp(buffer, "help") == 0){
      printf("List of commands:\n");
      printf("\tstop - stops the program completely\n");
      printf("\tlist - lists the IP of every peer connected\n");
      printf("\tlist IP - if a peer is connected, lists the files\n");
      printf("\t          shared by said peer\n");
      printf("\tget IP - lists all files broadcasted by the peer and asks\n");
      printf("\t          you the name of the file you want\n");
    }else if(strncmp(buffer, "list", 4) == 0){
      write(ctrl->udp_recv_pipe, "list", 4);
      if(buffer[4] != '\0'){
        err = inet_pton(AF_INET, buffer+5, &bf_addr);
        if(err == 1){
          write(ctrl->udp_recv_pipe, " ", 1);
          write(ctrl->udp_recv_pipe, &bf_addr.s_addr, sizeof(in_addr_t));
        }else if(err == 0){
          fprintf(stderr, "Invalid IPv4 address passed as an argument.\n");
        }else{
          fprintf(stderr, "Unspecified error occurred.\n");
        }
      }
      write(ctrl->udp_recv_pipe, "\0", 1);
      kill(ctrl->udp_recv_pid, SIGUSR1);
      sem_wait(sem);  // Wait for process to be over
    }else if(strncmp(buffer, "get ", 4) == 0){
      err = inet_pton(AF_INET, buffer+4, &bf_addr);
      if(err == 1){
        write(ctrl->udp_recv_pipe, "list ", 5);
        write(ctrl->udp_recv_pipe, &bf_addr.s_addr, sizeof(in_addr_t));
        write(ctrl->udp_recv_pipe, "\0", 1);
        kill(ctrl->udp_recv_pid, SIGUSR1);
        sem_wait(sem);  // Wait for process to be over

        printf("What file do you want to download?\n>");
        fgets(buffer, BUFFER_SIZE, stdin);

        p = buffer;
        end = buffer + BUFFER_SIZE;

        while(p != end){
          if(*p == '\n'){
            *p = '\0';
            break;
          }
          p++;
        }


        write(ctrl->udp_recv_pipe, "get ", 4);
        write(ctrl->udp_recv_pipe, &bf_addr.s_addr, sizeof(in_addr_t));
        write(ctrl->udp_recv_pipe, "\0", 1);
        kill(ctrl->udp_recv_pid, SIGUSR1);
        sem_wait(sem);  // Wait for process to be over
      }else if(err == 0){
        fprintf(stderr, "Invalid IPv4 address passed as an argument.\n");
      }else{
        fprintf(stderr, "Unspecified error occurred.\n");
      }
    }
    printf(">");
    fflush(stdout);
  }
}
