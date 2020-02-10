#include "init.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>

static const int true_val = 1;

int init_main_vars(struct main_var *vars){
  struct addrinfo hints, *list;
  int err;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;      // Only IPv4 allows broadcast
  hints.ai_socktype = SOCK_DGRAM; // UDP

  err = getaddrinfo(BCAST_ADDRESS, APP_PORT, &hints, &list);

  if(err != 0){
    fprintf(stderr, "failed to get broadcast address: %s\n",
        gai_strerror(err));
    return -1;
  }

  vars->bcast_addrlen = list->ai_addrlen;
  memcpy(&vars->bcast_addr, list->ai_addr, list->ai_addrlen);
  freeaddrinfo(list);
  return 0;
}

int init_udp_receive(int *sock_udp){
  struct addrinfo hints, *list, *rq;
  int err, sock;

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
    perror("init_stage3");
    return -1;
  }

  freeaddrinfo(list); // no longer needed
  *sock_udp = sock;
  return 0;
}

int init_udp_broadcast(int *sock_udp){
  struct addrinfo hints, *list, *rq;
  int err, sock, opt;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;      // Only IPv4 allows broadcast
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_flags = AI_PASSIVE;    // For wildcard IP address

  err = getaddrinfo(NULL, "0", &hints, &list);

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
    perror("init_stage2");
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

  *sock_udp = sock;
  return 0;
}

int init_tcp(int *sock_tcp){
  struct addrinfo hints, *list, *rq;
  int err, sock;

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

    err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     &true_val, sizeof(true_val));
    if(err != 0)
        continue;

    if(bind(sock, rq->ai_addr, rq->ai_addrlen) == 0)
      break;

    close(sock);
  }

  if(rq == NULL){
    perror("init_stage3");
    return -1;
  }

  freeaddrinfo(list);

  *sock_tcp = sock;
  return 0;
}

int init_tcp_listen(int sock_tcp, int *port){
  struct sockaddr_in sin;
  socklen_t len;
  int err, opt;

  opt = 1;

  err = setsockopt(sock_tcp, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if(err == -1){ perror("setsockopt"); return -1; }

  struct timeval timeout;
  memset(&timeout, 0, sizeof(struct timeval));
  timeout.tv_sec = 10;
  if(setsockopt(sock_tcp, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))
          || setsockopt(sock_tcp, SOL_SOCKET, SO_RCVTIMEO,
                        &timeout, sizeof(timeout))){
      perror("setsockopt");
      return -1;
  }

  err = listen(sock_tcp, MAX_TCP_CONN);
  if(err == -1) { perror("listen"); return -1; }

  len = sizeof(sin);
  err = getsockname(sock_tcp, (struct sockaddr*)&sin, &len);
  if(err == -1) { perror("getsockname"); return -1; }

  *port = ntohs(sin.sin_port);
  return 0;
}
