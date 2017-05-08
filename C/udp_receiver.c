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
#include <ifaddrs.h>

#include "udp_receiver.h"
#include "init.h"
#include "synchronize.h"
#include "broadcast.h"
#include "udp_receiver.h"
#include "p2p_cli.h"

#define BUFFER_SIZE 512
#define DEF_IGNOREOWNADDR 1

static int pipe_in = 0, pipe_out = 0, sock_udp_recv;
static struct peer_info* pinfo = NULL;
static sem_t* udp_sem = NULL;
static sem_t* op_sem = NULL;
static const char nullbyte = 0;

void cleanup_udp_recv(){
  if(pipe_in) close(pipe_in);
  if(pipe_out) close(pipe_out);
  free_peer_list_info(pinfo);
  if(udp_sem) sem_close(udp_sem);
  if(op_sem) sem_close(op_sem);
}

void usr_signal_handler(int signal){
  (void)signal;
  char buffer[BUFFER_SIZE];
  ssize_t nbyte;

  // TODO: Read parent command and execute it
  nbyte = read(pipe_in, buffer, BUFFER_SIZE);
  if(nbyte == -1){
    perror("read");
    fprintf(stderr, "UDP RECEIVER STOPPED WORKING!!!!\n");
    exit(EXIT_FAILURE);
  }

  remove_old_plist();

  if(strncmp(buffer, "list", 4) == 0){
    if(buffer[4] == '\0'){
      struct peer_info *curr = pinfo;
      if(curr == NULL){
        printf("No peers connected.\n");
      }else{
        printf("Listing all connected peers:\n");
        while(curr){
          inet_ntop(AF_INET, &curr->p_addr.sin_addr, buffer, INET_ADDRSTRLEN);
          printf("\t - %s\n", buffer);
          curr = curr->p_next;
        }
        printf("\n");
      }

    }else if(nbyte == 5 + sizeof(in_addr_t) + 1){
      struct peer_info *curr = pinfo;
      in_addr_t addr;

      memcpy(&addr, buffer+5, sizeof(in_addr_t));
      while(curr){
        if(memcmp(&curr->p_addr.sin_addr.s_addr, &addr,
                  sizeof(in_addr_t)) == 0){
          struct file_info *file_curr = curr->p_headfile;

          inet_ntop(AF_INET, &curr->p_addr.sin_addr, buffer, BUFFER_SIZE);
          printf("Listing all files from %s:\n", buffer);
          while(file_curr){
            printf("\t%s\n", file_curr->f_name);
            file_curr = file_curr->f_next;
          }
          printf("\n");
          write(pipe_out, "1", 1);
          int tcp_port = curr->p_tcp_port;
          char peer_tcp_port[10];
          int nbyte = sprintf(peer_tcp_port, "%d", tcp_port);
          write(pipe_out, peer_tcp_port, nbyte);
          write(pipe_out, &nullbyte, 1);
          break;
        }
        curr = curr->p_next;
      }
      if(curr == NULL){
        write(pipe_out, "0", 1);
      }
    }

  }else if(strncmp(buffer, "get", 4) == 0){

  }else{
    fprintf(stderr, "Got unknown command: \"%s\". Exiting...\n", buffer);
  }

  sem_post(op_sem); // Signal it is done
}

int start_udp_receiver(int* parent_fd_in, int* parent_fd_out){
  int pid, fd_in[2], fd_out[2], err;

  if(atexit(cleanup_udp_recv) != 0){
    perror("atexit");
    exit(EXIT_FAILURE);
  }

  if(signal(SIGUSR1, usr_signal_handler) == SIG_ERR){
    fprintf(stderr, "Could not set-up user signal handler.\n");
    exit(EXIT_FAILURE);
  }

  if(pipe(fd_in) == -1 || pipe(fd_out) == -1){
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  switch((pid = fork())){
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);

    case 0: /* child process */
      close(fd_in[1]); // Don't need to write to pipe
      pipe_in = fd_in[0];
      close(fd_out[0]); // Don't need to read from pipe
      pipe_out = fd_out[1];
      break;

    default: /* parent process */
      close(fd_in[0]); // Don't need to read from pipe
      *parent_fd_out = fd_in[1];
      close(fd_out[1]); // Don't need to write to pipe
      *parent_fd_in = fd_out[0];
      return pid;
  }

  if((udp_sem = sem_open(SEM_RECV_NAME, 0)) == SEM_FAILED
      || (op_sem = sem_open(SEM_OP_NAME, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  err = init_udp_receive(&sock_udp_recv);
  if(err == -1){
    fprintf(stderr, "udp_receiver: Could not create udp socket\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    struct sockaddr_storage temp;
    socklen_t temp_len;
    struct file_info *head_file;
    char buffer[UDP_DATAGRAM_MAX_SIZE];
    int nbytes, port;

    if((nbytes = recvfrom(sock_udp_recv, buffer, UDP_DATAGRAM_MAX_SIZE, 0,
      (struct sockaddr*)&temp, &temp_len)) > 0){
      int err;

      if(temp.ss_family != AF_INET) continue; // We only want IPv4

#if DEF_IGNOREOWNADDR
      if(is_own_address(&temp) == 0)
        continue; // This is our address, so we ignore it
#endif

      err = parse_datagram(buffer, UDP_DATAGRAM_MAX_SIZE, &head_file, &port);
      if(err != 0) continue;
      remove_old_plist();
      update_plist((struct sockaddr_in*)&temp, temp_len, head_file,
                    time(NULL), port);

    }else if(nbytes == -1){
      if(errno != EINTR){
        perror("recvfrom");
        exit(EXIT_FAILURE);
      }else{
        fprintf(stderr, "Interrupt detected!\n");
      }
    }
  }
}

int is_own_address(struct sockaddr_storage *temp){
  struct ifaddrs *head = NULL;
  struct ifaddrs *curr = NULL;
  int err;

  err = getifaddrs(&head);

  if(err != 0)
    return err;

  curr = head;
  while(curr){
    if(curr->ifa_addr->sa_family == AF_INET){
      if(memcmp(&((struct sockaddr_in*)curr->ifa_addr)->sin_addr,
                &((struct sockaddr_in*)temp)->sin_addr,
                sizeof(struct sockaddr_in)) == 0){
        freeifaddrs(head);
        return 0;
      }
    }
    curr = curr->ifa_next;
  }
  freeifaddrs(head);
  return -1;
}

void update_plist(struct sockaddr_in* np_addr, socklen_t np_addr_len,
                  struct file_info* head_file, time_t abstime,
                  int tcp_port){
  struct peer_info *curr, *prev;
  char peer_name[INET_ADDRSTRLEN];
  int lastItem = 0;

  remove_old_plist();
  prev = NULL;
  curr = pinfo;
  while(1){
    if(curr == NULL
        || memcmp(&curr->p_addr, np_addr, sizeof(struct sockaddr_in)) == 0){
      inet_ntop(AF_INET, &np_addr->sin_addr, peer_name, INET_ADDRSTRLEN);
      if(curr == NULL){
        fprintf(stderr, "Adding new peer: %s\n", peer_name);
        curr = (struct peer_info*) malloc(sizeof(struct peer_info));
        memset(curr, 0, sizeof(struct peer_info)); // Set all values to 0
        lastItem = 1;
      }else{
        if(curr->p_headfile)  // Shared folder may be empty
          freeflistinfo(curr->p_headfile);
        curr->p_headfile = NULL;
      }

      memcpy(&curr->p_addr, np_addr, sizeof(struct sockaddr_in));
      curr->p_addr_len = np_addr_len;
      curr->p_lastupdated = abstime;
      curr->p_headfile = head_file;
      curr->p_tcp_port = tcp_port;
      break;
    }
    prev = curr;
    curr = curr->p_next;
  }

  if(lastItem){
    if(prev != NULL) prev->p_next = curr;
    else pinfo = curr;
  }
}

void remove_old_plist(){
  struct peer_info *curr, *prev, *next;
  char peer_name[INET_ADDRSTRLEN];
  time_t abstime = time(NULL);

  prev = NULL;
  curr = pinfo;
  while(curr != NULL){
    next = curr->p_next;
    if(abstime - curr->p_lastupdated >= TIMEOUT){
      inet_ntop(AF_INET, &curr->p_addr.sin_addr, peer_name, INET_ADDRSTRLEN);
      fprintf(stderr, "Peer %s timed out. Removing from list.\n", peer_name);
      free_peer_info(curr);
      if(prev == NULL) pinfo = curr;  // If curr is the first one
      else prev->p_next = next;
    }else{
      prev = curr;
    }
    curr = next;
  }

  if(prev == NULL)  // If all entries were removed
    pinfo = NULL;
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
