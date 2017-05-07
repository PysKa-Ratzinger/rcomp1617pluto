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
#include "utils.h"
#include "synchronize.h"
#include "client.h"

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

    remove_newline(buffer);
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
      write(ctrl->udp_recv_pipe_out, "list", 4);
      if(buffer[4] != '\0'){
        err = inet_pton(AF_INET, buffer+5, &bf_addr);
        if(err == 1){
          write(ctrl->udp_recv_pipe_out, " ", 1);
          write(ctrl->udp_recv_pipe_out, &bf_addr.s_addr, sizeof(in_addr_t));
        }else if(err == 0){
          fprintf(stderr, "Invalid IPv4 address passed as an argument.\n");
        }else{
          fprintf(stderr, "Unspecified error occurred.\n");
        }
      }
      write(ctrl->udp_recv_pipe_out, "\0", 1);
      kill(ctrl->udp_recv_pid, SIGUSR1);
      sem_wait(sem);  // Wait for process to be over

    }else if(strncmp(buffer, "get ", 4) == 0){
      err = inet_pton(AF_INET, buffer+4, &bf_addr);
      if(err == 1){
        char file[BUFFER_SIZE];
        char location[BUFFER_SIZE];
        char answer[BUFFER_SIZE];
        write(ctrl->udp_recv_pipe_out, "list ", 5);
        write(ctrl->udp_recv_pipe_out, &bf_addr.s_addr, sizeof(in_addr_t));
        write(ctrl->udp_recv_pipe_out, "\0", 1);
        kill(ctrl->udp_recv_pid, SIGUSR1);
        sem_wait(sem);  // Wait for process to be over
        read(ctrl->udp_recv_pipe_in, answer, BUFFER_SIZE);
        if(answer[0] =='0'){
          printf("The specified peer either is not connected to the network\n"
                "or it's not broadcasting it's presence. Cannot list files.\n");
        }else if(answer[0] == '1'){
          unsigned short port = atoi(&answer[1]);
          printf("What file do you want to download?\n>");
          fflush(stdout);
          fgets(file, BUFFER_SIZE, stdin);
          remove_newline(file);

          printf("Where do you want to save the file? (please insert "
                "full path)\n>");
          fgets(location, BUFFER_SIZE, stdin);
          remove_newline(location);
          printf("TCP PORT = %u\n", port);
          start_file_transfer(bf_addr, port, file, location);

        }else{
          fprintf(stderr, "broadcast: Unknown response from receiver.\n");
        }
      }else if(err == 0){
        fprintf(stderr, "Invalid IPv4 address passed as an argument.\n");
      }else{
        fprintf(stderr, "Unspecified error occurred.\n");
      }
    }else{
        fprintf(stderr, "Type 'help' for a list of commands\n");
    }
    printf(">");
    fflush(stdout);
  }
}
