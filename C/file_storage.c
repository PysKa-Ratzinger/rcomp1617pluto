#include "file_storage.h"
#include "fork_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const int BUFFER_SIZE = 512;

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

int updatefs(struct file_storage* storage){
  int fd, len;
  off_t offset;
  ssize_t nbyte;
  struct file_info *curr, *next;
  char buffer[BUFFER_SIZE], folder[BUFFER_SIZE], *file;

  strcpy(folder, storage->f_folder);
  len = strlen(folder);
  folder[len] = '/';
  len++;
  folder[len] = '\0';
  file = folder + len;

  // TODO: Check stat of found file to see if it is a regular file

  offset = 0;
  curr = next = NULL;
  sprintf(buffer, "ls -p %s | grep -v /", storage->f_folder);
  fd = psystem(buffer);

  while(1){
    nbyte = pread(fd, buffer, 1, offset);
    if(nbyte == -1) break;

    if(buffer[offset] != '\n'){
      offset++;
    }else{
      buffer[offset] = 0;
      next->f_bytes = offset;

      next = malloc(sizeof(struct file_info));
      if(next == NULL){
        perror("malloc");
        return -1;
      }

      /* Takes care of uninitialized member variables */
      memset(next, 0, sizeof(struct file_info));

      next->f_name = malloc(next->f_bytes + 1); // +1 for null terminator
      if(next->f_name == NULL){
        free(next);
        return -1;
      }

      strcpy(next->f_name, buffer);
      if(curr == NULL)
        storage->f_headfile = next;
      else
        curr->f_next = next;
      curr = next;
    }
  }

  return 0;
}

int cleanfs(struct file_storage* storage){
  struct file_info *curr, *prev;

  curr = storage->f_headfile;
  if(curr == NULL)
    return -1;

  storage->f_headfile = NULL;
  do{
    prev = curr->f_next;
    free(curr->f_name);
    free(curr);
    curr = prev;
  }while(curr != NULL);

  return 0;
}

void freefsinfo(struct file_storage* storage){
  cleanfs(storage);
  free(storage->f_folder);
  free(storage);
}
