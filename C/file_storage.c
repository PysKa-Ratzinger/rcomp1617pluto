#include "file_storage.h"

static const int BUFFER_SIZE;

struct file_storage* createfsinfo(char* folder_name){
  struct file_storage *res;

  res = malloc(sizeof(struct file_storage));

  if(res == NULL)
    return NULL;

  res->f_folder = malloc(strlen(folder_name) + 1);  // +1 for null terminator

  if(res->f_folder == NULL){
    free(res);
    return NULL;
  }

  updatefs(res);
  return res;
}

void updatefs(struct file_storage* storage){
  int fd;
  off_t offset;
  ssize_t nbyte;
  struct file_info *curr, *next;
  char buffer[BUFFER_SIZE];

  offset = 0;
  head = next = NULL;
  fd = pipesystem("ls -p %s | grep -v /", storage->f_folder);

  while(1){
    nbyte = pread(fd, buffer, 1, offset);
    if(nbyte == -1) break;

    if(buffer[offset] == '\n'){
      buffer[offset] = 0;

      next = malloc(sizeof(file_info));
      if(next == NULL){
        perror("malloc");
        return -1;
      }

      /* Takes care of uninitialized member variables */
      memset(next, 0, sizeof(struct file_info));

      next->f_bytes = strlen(buffer);
      next->f_name = malloc(next->f_bytes + 1); // +1 for null terminator
      if(next->f_name == NULL){
        free(next);
        return -1;
      }

      man strcpy(next->f_name, buffer);
      if(curr == NULL)
        storage->f_headfile = next;
      else
        curr->f_next = next;
      curr = next;
    }else{
      offset++;
    }
  }

  return 0;
}

void cleanfs(struct file_storage* storage);

void freefsinfo(struct file_storage* storage);
