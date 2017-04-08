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

#include "init.h"
#include "fork_utils.h"
#include "file_storage.h"
#include "broadcast.h"
#include "utils.h"
#include "udp_receiver.h"

#define BUFFER_SIZE 256

static struct main_var vars;
static int broadcast_pid = 0;
static int udp_recv_pid = 0;
static int udp_recv_pipe = 0;

void ctrl_c_handler(int signal){
  (void)signal;
  // Will call the atexit functions of each separate process
  if(broadcast_pid && udp_recv_pid) // If is parent
    printf("Interrupt detected. Cleaning up...\n");
  exit(EXIT_SUCCESS);
}

void cleanup(){
  close(vars.sock_udp);
  close(vars.sock_ft);
  if(broadcast_pid) kill(broadcast_pid, SIGTERM);
  if(udp_recv_pid) kill(udp_recv_pid, SIGTERM);
  if(udp_recv_pipe) close(udp_recv_pipe);
}

int main(){
  sem_t* sem;
  char folder[BUFFER_SIZE];

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
  broadcast_pid = start_broadcast(&vars, folder, udp_recv_pid);

  printf("Press any key when you want the child to stop broadcasting.\n");
  getchar();
  stop_broadcast(broadcast_pid);

  return 0;
}
