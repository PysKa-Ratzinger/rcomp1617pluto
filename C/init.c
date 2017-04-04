#include "init.h"

void init(struct main_var *vars){
  struct addrinfo hints, *list;
  int err;

  // BROADCAST ADDRESS

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;      // Only IPv4 allows broadcast
  hints.ai_socktype = SOCK_DGRAM; // UDP

  err = getaddrinfo(BCAST_ADDRESS, APP_PORT, &hints, &list);

  if(err != 0){
    fprintf(stderr, "failed to get broadcast address: %s\n",
        gai_strerror(err));
    exit(EXIT_FAILURE);
  }

  vars->bcast_addrlen = list->ai_addrlen;
  memcpy(&vars->bcast_addr, list->ai_addr, list->ai_addrlen);
  freeaddrinfo(list);

  err = init_stage1(vars);

  if(err == -1){
    exit(EXIT_FAILURE);
  }
}

int init_stage1(struct main_var *vars){
  struct addrinfo hints, *list, *rq;
  int err, sock, opt;

  // BROADCAST LISTENING SOCKET

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;      // Only IPv4 allows broadcast
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_flags = AI_PASSIVE;    // For wildcard IP address

  err = getaddrinfo(NULL, APP_PORT, &hints, &list);

  if(err != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return -1;
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
    return -1;
  }

  freeaddrinfo(list); // no longer needed

  opt = 1;
  err = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

  if(err == -1){
    perror("setsockopt");
    close(sock);
    return -1;
  }

  vars->sock_udp = sock;

  err = init_stage2(vars);

  if(err == -1){
    close(sock);
    return -1;
  }

  return 0;
}

int init_stage2(struct main_var *vars){
  struct sockaddr_in sin;
  struct addrinfo hints, *list, *rq;
  socklen_t len;
  int err, sock, opt;

  // TCP FILE TRANSFER SOCKET

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;        // Since UDP only works on IPv4...
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE;

  err = getaddrinfo(NULL, "0", &hints, &list);  // 0 for wildcard port

  if(err != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return -1;
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
    return -1;
  }

  freeaddrinfo(list);

  for(;;){
    opt = 1;
    err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(err == -1){ perror("setsockopt"); break; }

    err = listen(sock, MAX_TCP_CONN);
    if(err == -1) { perror("listen"); break; }

    len = sizeof(sin);
    err = getsockname(sock, (struct sockaddr*)&sin, &len);
    if(err == -1) { perror("getsockname"); break; }

    vars->tcp_port = ntohs(sin.sin_port);

    break;  // Infinite loop runs only once
  }

  if(err == -1){
    close(sock);
    return -1;
  }

  vars->sock_ft = sock;

  return 0;
}
