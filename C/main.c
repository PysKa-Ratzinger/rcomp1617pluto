#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

#include "init.h"
#include "fork_utils.h"
#include "file_storage.h"
#include "broadcast.h"

#define BUFFER_SIZE 256

static struct main_var vars;
static int child_pid = 0;

void ctrl_c_handler(int signal){
  (void)signal;
  printf("\nInterrupt detected... cleaning up.\n");
  exit(EXIT_SUCCESS);
}

void cleanup(){
  close(vars.sock_udp);
  close(vars.sock_ft);
  freefsinfo(vars.storage);
  if(getpid() != child_pid){
    kill(child_pid, SIGTERM);
  }
  sem_unlink(SEM_SHM_NAME); // Just in case a crash happens
}

int main(){
  struct stat sb;
  int err;
  sem_t *sem;
  char folder[BUFFER_SIZE];

  if(signal(SIGINT, ctrl_c_handler) == SIG_ERR){
    fprintf(stderr, "Could not set-up interrupt handler.\n");
    exit(EXIT_FAILURE);
  }

  if(atexit(cleanup) != 0){
    perror("atexit");
    exit(EXIT_FAILURE);
  }

  init(&vars);

  err = 1;
  printf("Which folder would you like to share over the network?\n>");
  while(err){
    fflush(stdout);
    fgets(folder, BUFFER_SIZE, stdin);
    folder[strlen(folder)-1] = 0;

    if(stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode))
      break;  // Successfully read a folder

    printf("Please insert a valid path for a folder. "
            "Has to be a full path.\n>");
  }

  switch((child_pid = fork())){
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);

    case 0: /* child process */
      vars.storage = createfsinfo(folder);
      start_broadcast(&vars);
      exit(EXIT_SUCCESS);

    default: /* parent process */
      printf("Press any key when you want the child to stop broadcasting.\n");
      getchar();
      if((sem = sem_open(SEM_SHM_NAME, 0)) == SEM_FAILED){
        kill(child_pid, SIGTERM);
      }else{
        sem_post(sem);
        sem_close(sem);
      }
      wait(0);
      printf("Child stopped.\n");
      break;
  }

  return 0;
}
