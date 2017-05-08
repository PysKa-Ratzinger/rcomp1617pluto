#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>

#include "server.h"
#include "file_storage.h"
#include "init.h"

#define BUFFER_SIZE 4096
#define NUMBER_LIMIT 10

static char *my_folder;

void end(int signal){
  (void) signal;
  exit(EXIT_SUCCESS);
}

void handle_request(int fd, struct sockaddr_storage *peer_addr,
                    socklen_t peer_addr_len){
  char buffer[BUFFER_SIZE];
  unsigned int index = 0;
  ssize_t nbyte;

  if(inet_ntop(AF_INET, peer_addr, buffer, peer_addr_len) == NULL)
    handle_error("inet_ntop");

  fprintf(stderr, "(%d) Established connection with client %s.\n",
          getpid(), buffer);

  // ------------- Reads file name size ----------------------------

  while(1){
    nbyte = recv(fd, &buffer[index], 1, 0);
    if(nbyte <= 0){
      fprintf(stderr, "(%d) Connection with client terminated "
              "abruptly.\n", getpid());
      close(fd);
      exit(EXIT_FAILURE);
    }
    char c = buffer[index];
    index += nbyte;
    if(c != ':' && (c < '0' || c > '9')){
      fprintf(stderr, "(%d) Client sent illegal request. "
              "Closing connection...\n", getpid());
      close(fd);
      exit(EXIT_FAILURE);
    }else if(c == ':'){
      break;
    }else if(index >= NUMBER_LIMIT){
      fprintf(stderr, "(%d) Number too long. Closing "
              "connection...\n", getpid());
      close(fd);
      exit(EXIT_FAILURE);
    }
  }

  // ------------- Checks file name size --------------------------

  unsigned int filename_size = atoi(buffer);
  printf("(%d) Got number %d.\n", getpid(), filename_size);

  if(filename_size > FILENAME_SIZE-1){
    fprintf(stderr, "(%d) File name size is too long.\n", getpid());
    fprintf(stderr, "(%d) Closing connection...\n", getpid());
    close(fd);
    exit(EXIT_FAILURE);
  }

  unsigned int foldername_size = strlen(my_folder);
  unsigned int fullpath_size = foldername_size + 1 + filename_size;
  char *file = malloc(fullpath_size + 1);
  memcpy(file, my_folder, foldername_size);

  // ------------- Reads file name ----------------------------

  index = foldername_size;
  file[index++] = '/';
  while(index < fullpath_size){
    nbyte = recv(fd, &file[index], fullpath_size-index, 0);
    if(nbyte <= 0){
      fprintf(stderr, "(%d) Connection with client terminated "
              "abruptly.\n", getpid());
      free(file);
      close(fd);
      exit(EXIT_FAILURE);
    }
    index += nbyte;
  }
  file[index] = 0;

  // ------------- Checks file name --------------------------

  unsigned int i;
  for(i=foldername_size+1; i<index; i++){
    if(file[i] == '/'){
      fprintf(stderr, "(%d) Client sent illegal request.\n", getpid());
      fprintf(stderr, "(%d) Closing connection...\n", getpid());
      free(file);
      close(fd);
      exit(EXIT_FAILURE);
    }
  }

  // ------------- Checks file type --------------------------

  struct stat sb;
  if(!(stat(file, &sb) == 0 && S_ISREG(sb.st_mode))){
    fprintf(stderr, "(%d) Client requested an illegal file.\n", getpid());
    fprintf(stderr, "(%d) Closing connection...\n", getpid());
    free(file);
    close(fd);
    exit(EXIT_FAILURE);
  }

  // ------------- Opens file --------------------------

  FILE* file_fd = fopen(file, "r");
  free(file);

  if(file_fd == NULL){
    perror("open");
    fclose(file_fd);
    close(fd);
    exit(EXIT_FAILURE);
  }

  // ------------- Checks file size --------------------------

  fseek(file_fd, 0L, SEEK_END);
  size_t real_filesize = ftell(file_fd);
  fseek(file_fd, 0L, SEEK_SET);

  ssize_t res = snprintf(buffer, BUFFER_SIZE, "%lu:", real_filesize);
  if(res < 0){
    perror("snprintf");
    fclose(file_fd);
    close(fd);
    exit(EXIT_FAILURE);
  }

  // ------------- Sends file size --------------------------

  index = 0;
  while(index < res){
    nbyte = send(fd, &buffer[index], res - index, 0);
    if(nbyte <= 0){
      fprintf(stderr, "(%d) Connection with client terminated "
              "abruptly.\n", getpid());
      fclose(file_fd);
      close(fd);
      exit(EXIT_FAILURE);
    }
    index += nbyte;
  }

  // ------------- Sends file --------------------------

  while(real_filesize){
    size_t block_size = real_filesize < BUFFER_SIZE ?
                              real_filesize : BUFFER_SIZE;
    size_t buffer_size = fread(buffer, 1, block_size, file_fd);

    if(buffer_size == 0){
      if(feof(file_fd)){
        break;
      }else if(ferror(file_fd)){
        perror("fread");
        fclose(file_fd);
        close(fd);
        exit(EXIT_FAILURE);
      }
    }

    index = 0;
    while(index < buffer_size){
      nbyte = send(fd, &buffer[index], buffer_size-index, 0);
      if(nbyte <= 0){
        fprintf(stderr, "(%d) Connection with client terminated "
                "abruptly.\n", getpid());
        fclose(file_fd);
        close(fd);
        exit(EXIT_FAILURE);
      }
      index += nbyte;
    }

    real_filesize -= buffer_size;
  }
  fprintf(stderr, "(%d) File sent.\n", getpid());

  // ------------- Closes connection --------------------------

  fprintf(stderr, "(%d) Closing connection...\n", getpid());
  fclose(file_fd);
  close(fd);
}

int start_server(char* folder, int *tcp_port){
  int sock_tcp;
  init_tcp(&sock_tcp);
  init_tcp_listen(sock_tcp, tcp_port);

  my_folder = folder;

  int pid = fork();
  switch(pid){
    case -1: /* error */
      handle_error("server:fork");

    case 0: /* child behaviour */
      break;

    default: /* parent behaviour */
      close(sock_tcp);
      return pid;
  }

  // SERVER LOOP
  int tcp_connections = 0;
  while(1){
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    int fd;

    fd = accept(sock_tcp, (struct sockaddr*)&peer_addr, &peer_addr_len);
    if(fd == -1){
      if(errno != EINTR && errno != EAGAIN){
        handle_error("accept");
      }else{
        continue;
      }
    }

    if(tcp_connections == MAX_TCP_CONN){
      if(waitpid((pid_t)-1, 0, WNOHANG) > 0){
        tcp_connections--;
      }else{
        close(fd);
        printf("Max connections reached. Connection refused.\n");
        continue;
      }
    }

    tcp_connections++;
    switch(fork()){
      case -1:
        perror("fork");
        fprintf(stderr, "server: Could not respond to connection.\n");
        break;

      case 0: /* Child behaviour */
        handle_request(fd, &peer_addr, peer_addr_len);
        exit(EXIT_SUCCESS);

      default:  /* Parent behaviour */
        close(fd); // We don't need the file descriptor anymore
        break;
    }
  }

  exit(EXIT_SUCCESS);
}
