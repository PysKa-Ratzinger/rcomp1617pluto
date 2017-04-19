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

#define BCAST_ADDRESS "255.255.255.255"
#define APP_PORT "32033"
#define INTERVAL 4
#define TIMEOUT  6
#define MAX_TCP_CONN 50

struct main_var{
  struct sockaddr_in bcast_addr;  // broadcast address
  socklen_t bcast_addrlen;        // broadcast address length
  int tcp_port;                   // TCP port number
};

struct control_st{
  int parent_pid;         // Parent process (the one that controls)
  int broadcast_pid;      // Broadcast process pid
  int udp_recv_pid;       // UDP Receiver process pid
  int udp_recv_pipe_in;   // UDP Receiver request pipe
  int udp_recv_pipe_out;  // UDP Receiver response pipe
};

/**
 *    First of a series of init functions. Each calls the next one in the hopes
 *  that the next step initialization is successful. If an error occurs in any
 *  one of this functions, the function where the error originated shall
 *  return -1.
 *    In this case, the function that receives the error code -1 will know
 *  that it has to clean up every resource that will no longer be needed in
 *  order successfully exit the program. After the resources are cleaned this
 *  function shall as well return -1.
 *    The -1 "chain" will, like so, free all resources that were successfully
 *  allocated in consideration of those that were not.
 *
 *    This init function initializes the BROADCAST address where the UDP
 *  packets will be periodically sent.
 */
void init_main_vars(struct main_var *vars);

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
int init_tcp_port_number(int sock_tcp, int *port);
