#pragma once

/*******************************************************************
 *
 * UDP_RECEIVER.H -- UDP Receiver behaviour
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Define the behaviour of the udp_receiving process.
 */

 #include <time.h>

struct peer_info;

#include "init.hpp"
#include "file_storage.hpp"

#define NICK_MAX_DIGITS 15

struct peer_info{
  int                   p_version;
  struct sockaddr_in    p_addr;
  socklen_t             p_addr_len;
  unsigned short        p_id;
  unsigned short        p_tcp_port;
  char*                 p_nickname;
  struct file_info*     p_headfile;
  time_t                p_lastupdated;
  struct peer_info*     p_next;
};

/**
 *    Starts up a UDP receiving child process. This process will be
 *  always trying to read UDP packets from the predefined port.
 *    A pipe will also be created by which the child process will
 *  be able to receive orders from the parent process.
 *    In order to do that, however, the parent must first signal the
 *  child process with the SIGUSR1 signal, and then write to the pipe
 *  the desired order. This is designed to prevent a lock-down, where
 *  the child is not reading from the pipe and the parent tries to write
 *  into the pipe more information than the pipe can hold without anyone
 *  reading it.
 *
 *  RETURN:
 *    The pid of the process created shall be returned.
 */
int start_udp_receiver(int* fd_in, int* fd_out);

/**
 *    Checks to see if the address stored at 'temp' exists in the linked list
 *  started with 'ifap' passed as an argument.
 *    If it exists, 0 is returned, else -1 is returned.
 */
int is_own_address(struct sockaddr_storage *temp);

/**
 *    Updates the list of connected peers using a red-black
 */
void update_plist(struct sockaddr_in* np_addr, socklen_t np_addr_len,
                  struct peer_info* info, time_t abstime);

/**
 *    For every peer_info present in a list of known peer's, the current
 *  time is compared agaisnt the last update time. If the difference between
 *  the two is greater or equal to TIMEOUT, that entry is removed from the
 *  list.
 */
void remove_old_plist();

/**
 *    Frees the memory that was being used to hold the values of the
 *  peer info structure.
 */
void free_peer_info(struct peer_info* peer);

/**
 *    Frees the memory allocated to save the peer linked list.
 */
void free_peer_list_info(struct peer_info* head_peer);
