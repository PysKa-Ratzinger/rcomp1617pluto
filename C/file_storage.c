#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "file_storage.h"
#include "fork_utils.h"
#include "init.h"
#include "utils.h"
#include "broadcast.h"

static const int BUFFER_SIZE = 512;

struct file_storage* createfsinfo(char* folder_name){
  struct file_storage *res;

  res = (struct file_storage*)malloc(sizeof(struct file_storage));

  if(res == NULL)
    return NULL;

    // +1 for null terminator
  res->f_folder = (char*)malloc(strlen(folder_name) + 1);

  if(res->f_folder == NULL){
    free(res);
    return NULL;
  }

  strcpy(res->f_folder, folder_name);

  updatefs(res);
  return res;
}

int updatefs(struct file_storage* storage){
  struct stat sb;
  int fd, len;
  off_t offset;
  ssize_t nbyte;
  struct file_info *curr, *next;
  char buffer[BUFFER_SIZE], folder[BUFFER_SIZE], *file;

  strcpy(folder, storage->f_folder);
  len = strlen(folder);
  file = folder + len + 1;

  offset = 0;
  curr = next = NULL;
  sprintf(buffer, "ls -1 %s", storage->f_folder);
  fd = psystem(buffer);

  folder[len] = '/';
  while((nbyte = read(fd, file + offset, 1)) > 0){
    if(nbyte == -1){
      perror("read");
      break;
    }

    if(file[offset] == '\n'){
      file[offset] = 0;

      if(!(stat(folder, &sb) == 0 && S_ISREG(sb.st_mode))){
        offset = 0;
        continue;  // Read file is not a regular file
      }

      next = (struct file_info*)malloc(sizeof(struct file_info));
      if(next == NULL){
        perror("malloc");
        return -1;
      }

      /* Takes care of uninitialized member variables */
      memset(next, 0, sizeof(struct file_info));

      next->f_bytes = offset;
      // +1 for null terminator
      next->f_name = (char*)malloc(next->f_bytes + 1);
      if(next->f_name == NULL){
        free(next);
        return -1;
      }

      strcpy(next->f_name, file);
      if(curr == NULL)
        storage->f_headfile = next;
      else
        curr->f_next = next;
      curr = next;
      offset = 0;
    }else{
      offset++;
    }
  }

  close(fd);

  return 0;
}

int cleanfs(struct file_storage* storage){
  if(storage->f_headfile == NULL)
    return -1;

  freeflistinfo(storage->f_headfile);

  storage->f_headfile = NULL;
  return 0;
}

void freeflistinfo(struct file_info* flist){
  struct file_info *curr, *next;

  curr = flist;

  do{
    next = curr->f_next;
    free(curr->f_name);
    free(curr);
    curr = next;
  }while(curr != NULL);

}

void freefsinfo(struct file_storage* storage){
  if(storage == NULL) return;
  cleanfs(storage);
  if(storage->f_folder != NULL)
    free(storage->f_folder);
  free(storage);
}

void printfsinfo(struct file_storage* storage){
  struct file_info *curr;
  curr = storage->f_headfile;

  printf("Listing all files detected:\n");
  while(curr != NULL){
    printf("\t%s\n", curr->f_name);
    curr = curr->f_next;
  }
  printf("\n");
}


void construct_udp_data(struct main_var *vars, char* data,
                        struct file_storage* storage, size_t* data_len){
  size_t res = 0;
  struct file_info *curr = storage->f_headfile;

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
