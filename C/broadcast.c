#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>

#include "utils.h"
#include "file_storage.h"
#include "broadcast.h"
#include "init.h"

void start_broadcast(struct main_var *vars){
  struct timespec abs_timeout;
  size_t data_len;
  sem_t *sem;
  char data[UDP_DATAGRAM_MAX_SIZE];

  sem = sem_open(SEM_SHM_NAME, O_CREAT | O_EXCL, 0644, 0);
  while(1){
    int nbyte, s;
    construct_udp_data(vars, data, &data_len);
    nbyte = sendto(vars->sock_udp, data, data_len, 0,
                    (struct sockaddr*)&vars->bcast_addr, vars->bcast_addrlen);
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
      sem_close(sem);
      sem_unlink(SEM_SHM_NAME);
      return; // exit function
    }
  }
}

void construct_udp_data(struct main_var *vars, char* data, size_t* data_len){
  size_t res = 0;
  struct file_info *curr = (vars->storage)->f_headfile;

  res += sprintf(data, "PLUTO_v2;tport:%dd", vars->tcp_port);
  while(curr != NULL){
    if((res + num_places((unsigned short)curr->f_bytes) + 1
        + curr->f_bytes + 1) > UDP_DATAGRAM_MAX_SIZE){
      fprintf(stderr, "Could not broadcast all files. UDP max datagram "
                      "does not allow it\n");
      break;
    }else{
      res += sprintf(data + res, "%d:%s", curr->f_bytes, curr->f_name);
    }
    curr = curr->f_next;
  }
  res += sprintf(data + res, "e");

  *data_len = res;
}
