#pragma once

/*******************************************************************
 *
 * INIT.H -- Initialization library
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Initialize the program main variables
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define BCAST_ADDRESS "255.255.255.255"
#define APP_PORT "32033"
#define INTERVAL 30
#define TIMEOUT  45
#define MAX_TCP_CONN 50

struct main_var{
  struct sockaddr_in bcast_addr;  // broadcast address
  socklen_t bcast_addrlen;        // broadcast address length
  int sock_udp;                   // UDP socket
  int sock_ft;                    // TCP file transfer
  int tcp_port;                   // TCP port number
};

void init(struct main_var *vars);
int init_stage1(struct main_var *vars);
int init_stage2(struct main_var *vars);
