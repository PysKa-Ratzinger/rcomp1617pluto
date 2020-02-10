#pragma once

/*******************************************************************
 *
 * INIT.H -- Initialization library
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Initialize the program main variables
 */

#include <sys/socket.h>
#include <netinet/in.h>

#define handle_error(msg) \
  do{ perror(msg); exit(EXIT_FAILURE); } while(0)

#define BCAST_ADDRESS "255.255.255.255"
#define APP_PORT "32033"
#define UDP_DATAGRAM_MAX_SIZE 508 // Safe size
#define INTERVAL 30
#define TIMEOUT  45
#define MAX_TCP_CONN 5
#define CURR_VERSION 4

struct main_var{
  struct sockaddr_in bcast_addr;  // Broadcast address
  socklen_t bcast_addrlen;        // Broadcast address length
  char *nickname;                 // Machine Nickname
  int tcp_port;                   // TCP port number
};

struct control_st{
  int parent_pid;         // Parent process (the one that controls)
  int broadcast_pid;      // Broadcast process pid
  int server_pid;         // Broadcast process pid
  int udp_recv_pid;       // UDP Receiver process pid
  int udp_recv_pipe_in;   // UDP Receiver request pipe
  int udp_recv_pipe_out;  // UDP Receiver response pipe
};

/**   Initializes main variables structure
 */
int init_main_vars(struct main_var *vars);

/**   Initializes the UDP socket to be used in broadcast
 */
int init_udp_broadcast(int *sock_udp);

/**   Initializes the UDP socket to be used for listening broadcasts
 */
int init_udp_receive(int *sock_udp);

/**   Initializes the TCP socket
 */
int init_tcp(int *sock_tcp);

/**   Initializes the TCP port variable
 */
int init_tcp_listen(int sock_tcp, int *port);
