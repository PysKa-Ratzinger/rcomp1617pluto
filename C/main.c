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
#include "p2p_cli.h"

#define BUFFER_SIZE 256

static struct main_var vars;
static struct control_st ctrl;

void ctrl_c_handler(int signal){
  (void)signal;
  // Will call the atexit functions of each separate process
  if(getpid() == ctrl.parent_pid)
    printf("Interrupt detected. Cleaning up...\n");
  exit(EXIT_SUCCESS);
}

void cleanup(){
  close(vars.sock_udp);
  close(vars.sock_ft);
  if(ctrl.broadcast_pid) kill(ctrl.broadcast_pid, SIGTERM);
  if(ctrl.udp_recv_pid) kill(ctrl.udp_recv_pid, SIGTERM);
  if(ctrl.udp_recv_pipe) close(ctrl.udp_recv_pipe);
  sem_unlink(SEM_BCAST_NAME);
  sem_unlink(SEM_RECV_NAME);
  sem_unlink(SEM_OP_NAME);
}

int main(){
  sem_t* sem;
  char folder[BUFFER_SIZE];

  ctrl.parent_pid = getpid();
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
  sem = sem_open(SEM_OP_NAME, O_CREAT | O_EXCL, 0644, 0); sem_close(sem);

  init(&vars);
  read_folder(folder, BUFFER_SIZE);
  ctrl.udp_recv_pid = start_udp_receiver(&vars, &ctrl.udp_recv_pipe);
  fprintf(stderr, "Receiver process pid: %d\n", ctrl.udp_recv_pid);
  ctrl.broadcast_pid = start_broadcast(&vars, folder, ctrl.udp_recv_pid);
  fprintf(stderr, "Broadcaster process pid: %d\n", ctrl.broadcast_pid);

  start_cli(&ctrl);

  return 0;
}
