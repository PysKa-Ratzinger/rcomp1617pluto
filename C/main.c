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
#include <semaphore.h>
#include <fcntl.h>
#include <ifaddrs.h>

#include "init.h"
#include "fork_utils.h"
#include "file_storage.h"
#include "broadcast.h"
#include "utils.h"
#include "udp_receiver.h"

#define BUFFER_SIZE 256

static struct main_var vars;
static int parent_pid = 0;
static int broadcast_pid = 0;
static int udp_recv_pid = 0;
static int udp_recv_pipe = 0;

void ctrl_c_handler(int signal){
  (void)signal;
  // Will call the atexit functions of each separate process
  if(getpid() == parent_pid)
    printf("Interrupt detected. Cleaning up...\n");
  exit(EXIT_SUCCESS);
}

void cleanup(){
  close(vars.sock_udp);
  close(vars.sock_ft);
  if(broadcast_pid) kill(broadcast_pid, SIGTERM);
  if(udp_recv_pid) kill(udp_recv_pid, SIGTERM);
  if(udp_recv_pipe) close(udp_recv_pipe);
  sem_unlink(SEM_BCAST_NAME);
  sem_unlink(SEM_RECV_NAME);
}

void start_cli(){
  int err;
  char buffer[BUFFER_SIZE];
  struct in_addr bf_addr;

  printf(">");
  fflush(stdout);
  while(fgets(buffer, BUFFER_SIZE, stdin)){
    if(strcmp(buffer, "stop") == 0){
      printf("Stopping the program.\n");
      stop_broadcast(broadcast_pid);
      return;
    }else if(strcmp(buffer, "help") == 0){
      printf("List of commands:\n");
      printf("\tstop - stops the program completely\n");
      printf("\tlist - lists the IP of every peer connected\n");
      printf("\tlist IP - if a peer is connected, lists the files\n");
      printf("\t          shared by said peer");
      printf("\tget IP FILE LOCATION - if IP is the ip of a conected peer\n");
      printf("\t                       and FILE exists in the list of\n");
      printf("\t                       shared files of that peer, attempts\n");
      printf("\t                       to download FILE into LOCATION\n");
    }else if(strncmp(buffer, "list", 4) == 0){
      if(buffer[4] == '\0'){
        // TODO: list
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

int main(){
  sem_t* sem;
  char folder[BUFFER_SIZE];

  parent_pid = getpid();
  if(signal(SIGINT, ctrl_c_handler) == SIG_ERR){
    fprintf(stderr, "Could not set-up interrupt handler.\n");
    exit(EXIT_FAILURE);
  }

  if(atexit(cleanup) != 0){
    perror("atexit");
    exit(EXIT_FAILURE);
  }

  sem = sem_open(SEM_BCAST_NAME, O_CREAT | O_EXCL, 0644, 0); sem_close(sem);
  sem = sem_open(SEM_RECV_NAME, O_CREAT | O_EXCL, 0644, 0); sem_close(sem);

  init(&vars);
  read_folder(folder, BUFFER_SIZE);
  udp_recv_pid = start_udp_receiver(&vars, &udp_recv_pipe);
  fprintf(stderr, "Receiver process pid: %d\n", udp_recv_pid);
  broadcast_pid = start_broadcast(&vars, folder, udp_recv_pid);
  fprintf(stderr, "Broadcaster process pid: %d\n", broadcast_pid);

  start_cli();

  return 0;
}
