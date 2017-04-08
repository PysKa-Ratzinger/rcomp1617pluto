#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "udp_receiver.h"
#include "init.h"
#include "broadcast.h"
#include "udp_receiver.h"

static int pipe_fd = 0;
static struct peer_info* pinfo = NULL;
static sem_t* sem = NULL;

void cleanup_udp_recv(){
  if(pipe_fd) close(pipe_fd);
  free_peer_list_info(pinfo);
  if(sem) sem_close(sem);
  sem_unlink(SEM_RECV_NAME);
}

void usr_signal_handler(int signal){
  (void)signal;

  // TODO: Read parent command and execute it
}

void synchronize(int signal){
  (void)signal;
  sem_wait(sem);  // Wait for the go signal
}

int start_udp_receiver(struct main_var* vars, int* file_descriptor){
  int pid, fd[2];

  if(atexit(cleanup_udp_recv) != 0){
    perror("atexit");
    exit(EXIT_FAILURE);
  }

  if(signal(SIGUSR1, usr_signal_handler) == SIG_ERR
      || signal(SIGUSR2, synchronize) == SIG_ERR){
    fprintf(stderr, "Could not set-up user signal handler.\n");
    exit(EXIT_FAILURE);
  }

  if(pipe(fd) == -1){
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  switch((pid = fork())){
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);

    case 0: /* child process */
      close(fd[1]); // Don't need to write to pipe
      pipe_fd = fd[0];
      break;

    default: /* parent process */
      close(fd[0]); // Don't need to read from pipe
      *file_descriptor = fd[1];
      return pid;
  }

  if((sem = sem_open(SEM_RECV_NAME, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  while(1){
    struct sockaddr_storage temp;
    socklen_t temp_len;
    char buffer[UDP_DATAGRAM_MAX_SIZE];
    int nbytes;

    if((nbytes = recvfrom(vars->sock_udp, buffer, UDP_DATAGRAM_MAX_SIZE, 0,
      (struct sockaddr*)&temp, &temp_len)) > 0){
      char peer_name[INET_ADDRSTRLEN];

      if(temp.ss_family != AF_INET) continue; // We only want IPv4

      // TODO: Precess the obtained result
      // update_plist((struct sockaddr_in*)&temp, temp_len, NULL, time(NULL));
      inet_ntop(AF_INET, &((struct sockaddr_in*)&temp)->sin_addr,
                peer_name, INET_ADDRSTRLEN);

      // TODO: Remove this debug line
      fprintf(stderr, "Read: %s from %s\n", buffer, peer_name);

    }else if(nbytes == -1 && errno != EINTR){
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
  }
}

void update_plist(struct sockaddr_in* np_addr, socklen_t np_addr_len,
                  struct file_info* head_file, time_t abstime){
  remove_old_plist();
  if(pinfo == NULL){
    pinfo = (struct peer_info*)malloc(sizeof(struct peer_info));
    memset(pinfo, 0, sizeof(struct peer_info)); // Set all values to 0
    memcpy(&pinfo->p_addr, np_addr, sizeof(struct sockaddr_in));
    pinfo->p_addr_len = np_addr_len;
    pinfo->p_lastupdated = abstime;
  }else{
    struct peer_info* curr = pinfo;
    do{
      if(memcmp(&curr->p_addr, np_addr, sizeof(struct sockaddr_in)) == 0){
        memcpy(&curr->p_addr, np_addr, sizeof(struct sockaddr_in));
        curr->p_addr_len = np_addr_len;
        curr->p_lastupdated = abstime;
        freeflistinfo(curr->p_headfile);
        curr->p_headfile = head_file;
        break;
      }
    }while(curr->p_next != NULL);
  }
}

void remove_old_plist(){
  struct peer_info *curr, *prev, *next;
  time_t abstime = time(NULL);

  prev = NULL;
  curr = pinfo;
  while(curr != NULL){
    next = curr->p_next;
    if(abstime - curr->p_lastupdated >= TIMEOUT){
      free_peer_info(curr);
      if(prev == NULL) pinfo = curr;
      else prev->p_next = next;
    }else{
      prev = curr;
    }
    curr = next;
  }
}

void free_peer_info(struct peer_info* peer){
  if(peer == NULL) return;
  if(peer->p_headfile != NULL) freeflistinfo(peer->p_headfile);
  free(peer);
}

void free_peer_list_info(struct peer_info* head_peer){
  struct peer_info *curr, *next;

  curr = head_peer;
  while(curr != NULL){
    next = curr->p_next;
    free_peer_info(curr);
    curr = next;
  }
}
