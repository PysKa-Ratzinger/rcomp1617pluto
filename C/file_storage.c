#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/wait.h>

#include "file_storage.h"
#include "fork_utils.h"
#include "init.h"
#include "utils.h"
#include "broadcast.h"

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
  char buffer[FILENAME_SIZE], folder[FILENAME_SIZE], *file;

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
  wait(0);

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
  printflistinfo(storage->f_headfile);
}

void printflistinfo(struct file_info* head_file){
  struct file_info *curr = head_file;

  printf("Listing all files detected:\n");
  while(curr != NULL){
    printf("\t%s\n", curr->f_name);
    curr = curr->f_next;
  }
  printf("\n");
}

struct udp_datagrams* construct_udp_data(struct main_var *vars,
                                         struct file_storage* storage,
					                               unsigned short id){
  size_t res;
  struct file_info *curr = storage->f_headfile;
  struct udp_datagrams *res_data, *curr_udp;
  int has_next = 1;

  res_data = (struct udp_datagrams*) malloc(sizeof(struct udp_datagrams));
  curr_udp = res_data;

  while(has_next){
    res = 0;
    has_next = 0;
    memset(curr_udp, 0, sizeof(struct udp_datagrams));
    curr_udp->u_data = (char*) malloc(UDP_DATAGRAM_MAX_SIZE);
    // UDP Datagram initializer
    res += sprintf(curr_udp->u_data, "PLUTO_v%u;%hu;tport:%d;nick:%s;d",
                   CURR_VERSION, id, vars->tcp_port, vars->nickname);

    // Filling with file information
    while(curr != NULL){
      if((res + num_places((unsigned short)curr->f_bytes) + 1
          + curr->f_bytes + 1) > UDP_DATAGRAM_MAX_SIZE){
        has_next = 1;
        break;
      }else{
        res += sprintf(curr_udp->u_data + res, "%d:%s", curr->f_bytes,
                        curr->f_name);
      }
      curr = curr->f_next;
    }
    res += sprintf(curr_udp->u_data + res, "e");
    curr_udp->u_len = res;

    if(has_next){
      curr_udp->u_next = (struct udp_datagrams*)
                          malloc(sizeof(struct udp_datagrams));
      curr_udp = curr_udp->u_next;
    }
  }
  return res_data;
}

void free_udp_datagrams(struct udp_datagrams* target){
  struct udp_datagrams *curr, *next;

  curr = target;
  while(curr){
    next = curr->u_next;
    free(curr->u_data);
    free(curr);
    curr = next;
  }
}

int parse_datagram(char *buffer, size_t buffer_max_size,
                  struct peer_info *peer_info){
  enum state {HEADER, VERSION, ID, PORT, NICK, DICT_START,
              DICT, DICT_END, ENTRY, END};
  int i, version, id, tport, nbyte, err = 0;
  struct file_info *head, *curr;
  enum state st = HEADER;
  char name[UDP_DATAGRAM_MAX_SIZE];
  char c, *p, *end, *nick;

  p = buffer;
  end = buffer + buffer_max_size;
  head = curr = NULL;

  while(st != END){
    switch(st){
      case HEADER:
          // Check header
        if(p + 7 >= end){ err = 1; break;};
        if(strncmp(p, "PLUTO_v", 7) != 0){
          fprintf(stderr, "parse_datagram: Datagram application protocol does "
                          "not have correct header.\n");
          err = 2;
          break;
        }
        p += 7;
        st = VERSION;
        break;

      case VERSION:
          // Check version
        if(p == end){ err = 1; break;};
        c = *p;
        i = 0;
        version = 0;
        while(i < 20 && c >= '0' && c <= '9'){
          version = version * 10 + c - '0';
          p++; i++;
          if(p == end){ err = 1; break;};
          c = *p;
        }
        if(err) break;

        if(i == 0){
          fprintf(stderr, "parse_datagram: Datagram version does "
                          "not exist. Skipping...\n");
          err = 2;
          break;
        }else if(i == 20){
          fprintf(stderr, "parse_datagram: Datagram "
                          "version has too many digits.\n");
          err = 2;
          break;
        }

        if(version > CURR_VERSION || version <= 0){
          fprintf(stderr, "parse_datagram: Datagram application protocol has "
                          "unknown version at %s. Skipping...\n", p);
          err = 2;
          break;
        }
        if(*p != ';'){
          fprintf(stderr, "parse_datagram: Expected ';' after version. "
                          "Skipping...");
          err = 2;
          break;
        }
        p++;

        if(version >= 4){
          st = ID;
        }else{
          freeflistinfo(peer_info->p_headfile);
          peer_info->p_headfile = NULL;
          st = PORT;
        }
        break;

      case ID:
          // Parse id
        c = *p;
        i = 0;
        id = 0;
        while(i < 20 && c >= '0' && c <= '9'){
          id = id * 10 + c - '0';
          p++; i++;
          if(p == end){ err = 1; break;};
          c = *p;
        }
        if(err) break;

        if(i == 20){
          fprintf(stderr, "parse_datagram: Datagram id "
                          "number has too many digits.\n");
          err = 2;
          break;
        }

        if(id < 0 || id > USHRT_MAX){
          fprintf(stderr, "parse_datagram: Specified id number is out of "
                          "bounds. Skipping...\n");
          err = 2;
          break;
        }

        if(*p != ';'){
          fprintf(stderr, "parse_datagram: Expected ';' after id number. "
                          "Skipping...");
          err = 2;
          break;
        }
        p++;

        if(id == peer_info->p_id){
          head = peer_info->p_headfile;
          curr = head;
          if(curr){
            while(curr->f_next)
              curr = curr->f_next;
          }
        }else if(peer_info->p_headfile){
          freeflistinfo(peer_info->p_headfile);
          peer_info->p_headfile = NULL;
        }

        peer_info->p_id = id;

        st = PORT;
        break;

      case PORT:
          // Check port header
        if(p + 6 >= end){ err = 1; break;};
        if(strncmp(p, "tport:", 6) != 0){
          fprintf(stderr, "parse_datagram: Datagram application protocol does "
                          "not have correct header.\n");
          err = 2;
          break;
        }
        p += 6;

          // Parse port
        c = *p;
        i = 0;
        tport = 0;
        while(i < 20 && c >= '0' && c <= '9'){
          tport = tport * 10 + c - '0';
          p++; i++;
          if(p == end){ err = 1; break;};
          c = *p;
        }
        if(err) break;

        if(i == 20){
          fprintf(stderr, "parse_datagram: Datagram port "
                          "number has too many digits.\n");
          err = 2;
          break;
        }

        if(tport <= 0 || tport > USHRT_MAX){
          fprintf(stderr, "parse_datagram: Specified TCP port is out of "
                          "bounds. Skipping...\n");
          err = 2;
          break;
        }

        if(*p != ';'){
          fprintf(stderr, "parse_datagram: Expected ';' after port number. "
                          "Skipping...");
          err = 2;
          break;
        }
        p++;

        if(version >= 3){
            st = NICK;
        }else{
            st = DICT_START;
        }
        break;

      case NICK:
          // Check nickname header
        if(p + 5 >= end){ err = 1; break;};
        if(strncmp(p, "nick:", 5) != 0){
          fprintf(stderr, "parse_datagram: Datagram application protocol does "
                          "not have correct header.\n");
          err = 2;
          break;
        }
        p += 5;

        // Check nick size (variable i)
        c = *p;
        i = 0;
        while(i < NICK_MAX_DIGITS && c != '\0' && c != ';'){
          p++; i++;
          if(p == end){ err = 1; break;};
          c = *p;
        }
        if(err) break;

        if(c != ';'){
            fprintf(stderr, "parse_datagram: Nickname has more than 15 "
                    "characters. Skipping...\n");
            err = 2;
            break;
        }

        // Process nick
        nick = (char*)malloc(i+1);
        memcpy(nick, p-i, i);
        nick[i] = '\0';

        p++;
        st = DICT_START;
        break;

      case DICT_START:
        if(p == end){ err = 1; break;};
        if(*p != 'd'){
          fprintf(stderr, "parse_datagram: Expected 'd' (Dictionary) after "
                          "port number. Skipping...\n");
          err = 2;
          break;
        }
        p++;
        st = DICT;
        break;

      case DICT:
        if(p == end){ err = 1; break;};
        c = *p;
        if(c == 'e'){
          st = DICT_END;
        }else if(c >= '0' && c <= '9'){
          st = ENTRY;
        }else{
          err = 1;
        }
        break;

      case ENTRY:
          // Parse name size
        c = *p;
        i = 0;
        nbyte = 0;
        while(i < 20 && c >= '0' && c <= '9'){
          nbyte = nbyte * 10 + c - '0';
          p++; i++;
          if(p == end){ err = 1; break;};
          c = *p;
        }
        if(err) break;

        if(i == 20){
          fprintf(stderr, "parse_datagram: Name byte size "
                          "number has too many digits.\n");
          err = 2;
          break;
        }

        if(nbyte <= 0 || nbyte > UDP_DATAGRAM_MAX_SIZE){
          fprintf(stderr, "parse_datagram: Specified name byte size is out "
                          "of bounds. Skipping...\n");
          err = 2;
          break;
        }

        if(*p != ':'){
          fprintf(stderr, "parse_datagram: Expected ':' after name byte "
                          "size. Skipping...\n");
          err = 2;
          break;
        }
        p++;

        if(p + nbyte >= end){ err = -1; break; }
        strncpy(name, p, nbyte);
        p += nbyte;

        if(curr == NULL){
          curr = (struct file_info*)malloc(sizeof(struct file_info));
          head = curr;
        }else{
          curr->f_next = (struct file_info*)malloc(sizeof(struct file_info));
          curr = curr->f_next;
        }
        memset(curr, 0, sizeof(struct file_info));
        curr->f_bytes = nbyte;
        curr->f_name = (char*)malloc(nbyte+1);
        strncpy(curr->f_name, name, nbyte);
        curr->f_name[nbyte] = '\0';

        st = DICT;
        break;

      case DICT_END:
        peer_info->p_headfile = head;
        peer_info->p_tcp_port = tport;
        peer_info->p_nickname = nick;
        st = END;
        break;

      case END: break;  // Do nothing
    }
    if(err != 0){
      if(head) freeflistinfo(head);
      if(nick) free(nick);
      if(err == 1)
        fprintf(stderr, "parse_datagram: Buffer too small or message to large. "
                        "Skippin...\n");
      return -1;
    }
  }

  return 0;
}
