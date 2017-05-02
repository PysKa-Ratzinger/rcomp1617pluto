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
#include "synchronize.h"
#include "broadcast.h"
#include "utils.h"
#include "udp_receiver.h"
#include "p2p_cli.h"
#include "server.h"

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
  if(ctrl.broadcast_pid) kill(ctrl.broadcast_pid, SIGTERM);
  if(ctrl.udp_recv_pid) kill(ctrl.udp_recv_pid, SIGTERM);
  if(ctrl.udp_recv_pipe_in) close(ctrl.udp_recv_pipe_in);
  if(ctrl.udp_recv_pipe_out) close(ctrl.udp_recv_pipe_out);
  sem_unlink(SEM_BCAST_NAME);
  sem_unlink(SEM_RECV_NAME);
  sem_unlink(SEM_OP_NAME);
}

int main(){
  sem_t* sem[3];
  char folder[BUFFER_SIZE];

  ctrl.parent_pid = getpid();
  if(signal(SIGINT, ctrl_c_handler) == SIG_ERR)
    handle_error("signal");

  if(atexit(cleanup) != 0)
    handle_error("atexit");

  sem[0] = sem_open(SEM_BCAST_NAME, O_CREAT | O_EXCL, 0644, 0);
  sem[1] = sem_open(SEM_RECV_NAME, O_CREAT | O_EXCL, 0644, 0);
  sem[2] = sem_open(SEM_OP_NAME, O_CREAT | O_EXCL, 0644, 0);

  int i;
  for(i=0; i<3; i++){
    if(sem[i] == SEM_FAILED)
      handle_error("sem_open");
    sem_close(sem[i]);
  }

  if(init_main_vars(&vars) == -1){
    printf("[ERROR] Initialization of main variables failed. Quitting...\n");
    exit(EXIT_FAILURE);
  }
  read_folder(folder, BUFFER_SIZE);
  ctrl.server_pid = start_server(folder, &vars.tcp_port);
  ctrl.udp_recv_pid = start_udp_receiver(&ctrl.udp_recv_pipe_in,
                                          &ctrl.udp_recv_pipe_out);
  fprintf(stderr, "Receiver process pid: %d\n", ctrl.udp_recv_pid);
  ctrl.broadcast_pid = start_broadcast(&vars, folder);
  fprintf(stderr, "Broadcaster process pid: %d\n", ctrl.broadcast_pid);

  start_cli(&ctrl);

  return 0;
}
