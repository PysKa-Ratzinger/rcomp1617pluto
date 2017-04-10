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
#define INTERVAL 1
#define TIMEOUT  45
#define MAX_TCP_CONN 50

struct main_var{
  struct sockaddr_in bcast_addr;  // broadcast address
  socklen_t bcast_addrlen;        // broadcast address length
  struct sockaddr_in own_addr;    // own address
  socklen_t own_addrlen;          // own address length
  int sock_udp;                   // UDP socket
  int sock_ft;                    // TCP file transfer
  int tcp_port;                   // TCP port number
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
void init(struct main_var *vars);

/**   Initializes the UDP socket
 */
int init_stage1(struct main_var *vars);

/**   Initializes the TCP socket
 */
int init_stage2(struct main_var *vars);

/**   Initializes the TCP port variable
 */
int init_stage3(struct main_var *vars);

/**   Gets own IP address
 */
int init_stage4(struct main_var *vars);
