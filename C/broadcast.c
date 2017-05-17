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
#include "synchronize.h"
#include "file_storage.h"
#include "broadcast.h"
#include "init.h"
#include "udp_receiver.h"

static struct file_storage *storage;
static struct control_st *m_ctrl;
static sem_t *sem;
static int sock_udp_send;
static unsigned short static_id = 0;

void cleanup_broadcast(){
  kill(m_ctrl->parent_pid, SIGTERM);
  freefsinfo(storage);
  sem_close(sem);
}

int start_broadcast(struct main_var *vars, char *folder,
                  struct control_st *ctrl){
  struct timespec abs_timeout;
  int pid, err;

  m_ctrl = ctrl;
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

  if((sem = sem_open(SEM_BCAST_NAME, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  err = init_udp_broadcast(&sock_udp_send);
  if(err == -1){
    fprintf(stderr, "broadcast: Could not create udp socket\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    int nbyte, s;
    struct udp_datagrams *data, *curr;

    storage = createfsinfo(folder);

    // SEND UDP DATAGRAMS
    data = construct_udp_data(vars, storage, ++static_id);
    curr = data;
    while(curr){
      nbyte = sendto(sock_udp_send, curr->u_data, curr->u_len, 0,
                      (struct sockaddr*)&vars->bcast_addr, vars->bcast_addrlen);
      if(nbyte == -1){
        perror("sendto");
        exit(EXIT_FAILURE);
      }
      curr = curr->u_next;
    }

    free_udp_datagrams(data);

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

  if((sem = sem_open(SEM_BCAST_NAME, 0)) != SEM_FAILED){
    sem_post(sem);
    sem_close(sem);
  }else{
    kill(pid, SIGTERM);
  }
  wait(0);
  printf("Child stopped.\n");
}
