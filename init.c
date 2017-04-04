#include "init.h"

void init(struct main_var *vars){
  struct addrinfo hints;
  struct addrinfo *list, *rq;
  int err, sock, opt;
  char buffer[BUFFER_SIZE];

  // BROADCAST ADDRESS

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;      // Only IPv4 allows broadcast
  hints.ai_socktype = SOCK_DGRAM; // UDP

  err = getaddrinfo(BCAST_ADDRESS, APP_PORT, &hints, &list);

  if(err != 0){
    fprintf(stderr, "failed to get broadcast address: %s\n",
        gai_strerror(err));
    freeaddrinfo(list);
    exit(EXIT_FAILURE);
  }

  vars->bcast_addrlen = list->ai_addrlen;
  memcpy(&vars->bcast_addr, list->ai_addr, list->ai_addrlen);
  freeaddrinfo(list);

  // BROADCAST LISTENING SOCKET

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;      // Only IPv4 allows broadcast
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_flags = AI_PASSIVE;    // For wildcard IP address

  err = getaddrinfo(NULL, APP_PORT, &hints, &list);

  if(err != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    freeaddrinfo(list);
    exit(EXIT_FAILURE);
  }

  for(rq = list; rq != NULL; rq = rq->ai_next){
    sock = socket(rq->ai_family, rq->ai_socktype, rq->ai_protocol);
    if(sock == -1)
      continue;

    if(bind(sock, rq->ai_addr, rq->ai_addrlen) == 0)
      break;    // success

    close(sock);
  }

  if(rq == NULL){ // No address succeded
    fprintf(stderr, "Could not bind.\n");
    exit(EXIT_FAILURE);
  }

  opt = 1;
  err = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

  if(err == -1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  vars->sock_udp = sock;
  freeaddrinfo(list);

  // TCP FILE TRANSFER SOCKET

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;        // Since UDP only works on IPv4...
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE;

  err = getaddrinfo(NULL, "0", &hints, &list);  // 0 for wildcard port

  if(err != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    close(vars->sock_udp);
    freeaddrinfo(list);
    exit(EXIT_FAILURE);
  }

  for(rq = list; rq != NULL; rq = rq->ai_next){
    sock = socket(rq->ai_family, rq->ai_socktype, rq->ai_protocol);
    if(sock == -1)
      continue;

    if(bind(sock, rq->ai_addr, rq->ai_addrlen) == 0)
      break;

    close(sock);
  }

  if(rq == NULL){
    fprintf(stderr, "Could not bind.\n");
    close(vars->sock_udp);
    freeaddrinfo(list);
    exit(EXIT_FAILURE);
  }

  opt = 1;
  err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if(err == -1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  err = getnameinfo(rq->ai_addr, rq->ai_addrlen, NULL, 0,
                    buffer, BUFFER_SIZE, NI_NUMERICSERV);

  if(err != 0){
    perror("getnameinfo");
    exit(EXIT_FAILURE);
  }

  vars->tcp_port = atoi(buffer);
  vars->sock_ft = sock;
  freeaddrinfo(list);
}

