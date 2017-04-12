#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>

#include "utils.h"
#include "file_storage.h"
#include "broadcast.h"
#include "init.h"
#include "udp_receiver.h"

static struct file_storage* storage;
static sem_t *sem, *sem_read;

void cleanup_broadcast(){
  freefsinfo(storage);
  sem_close(sem);
  sem_close(sem_read);
}

int start_broadcast(struct main_var *vars, char *folder, int udp_recv_pid){
  struct timespec abs_timeout;
  size_t data_len;
  int pid, sem_value;
  char data[UDP_DATAGRAM_MAX_SIZE];

  switch((pid = fork())){
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);

    case 0:  break;  /* child process */
    default: return pid; /* parent process */
  }

  if(atexit(cleanup_broadcast) != 0){
    perror("atexit");
    exit(EXIT_FAILURE);
  }

  if((sem = sem_open(SEM_BCAST_NAME, 0)) == SEM_FAILED
      || (sem_read = sem_open(SEM_RECV_NAME, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  while(1){
    int nbyte, s;

    storage = createfsinfo(folder);
    construct_udp_data(vars, data, storage, &data_len);
    kill(udp_recv_pid, SIGUSR2);
    nbyte = sendto(vars->sock_udp, data, data_len, 0,
                    (struct sockaddr*)&vars->bcast_addr, vars->bcast_addrlen);
    sem_getvalue(sem_read, &sem_value);
    if(sem_value == 0) sem_post(sem_read);
    else fprintf(stderr, "Broadcaster: Semaphore not at starting value...\n");
    if(nbyte == -1){
      perror("sendto");
      exit(EXIT_FAILURE);
    }

    if(clock_gettime(CLOCK_REALTIME, &abs_timeout) == -1){
      perror("clock_gettime");
      exit(EXIT_FAILURE);
    }

    abs_timeout.tv_sec += INTERVAL;

    while((s = sem_timedwait(sem, &abs_timeout)) == -1 && errno == EINTR)
      continue; /* Restart if interrupted by handler */

    if(s == -1){
      if(errno == ETIMEDOUT){
        continue; /* successfully waited INTERVAL seconds */
      }else{
        perror("sem_timedwait");
      }
    }else{
      // sem_post happened somewhere. This means it should stop broadcasting
      exit(EXIT_SUCCESS);
    }
  }

  exit(EXIT_SUCCESS);
}

void stop_broadcast(int pid){
  sem_t *sem;

  if((sem = sem_open(SEM_BCAST_NAME, 0)) == SEM_FAILED){
    kill(pid, SIGTERM);
  }else{
    sem_post(sem);
    sem_close(sem);
  }
  wait(0);
  printf("Child stopped.\n");
}
