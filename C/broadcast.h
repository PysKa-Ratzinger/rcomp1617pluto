#pragma once

#include "init.h"

#define UDP_DATAGRAM_MAX_SIZE 65507

/*******************************************************************
 *
 * BROADCAST.H -- Broadcast behaviour library
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Defines the way broadcasting works
 */

/**
 *    Initiates broadcasting to the network about what the TCP port is
 *  and also what files are available for download.
 */
void start_broadcast(struct main_var *vars, char *folder);

/**
 *    Stops the udp packet broadcasting
 */
void stop_broadcast();

/**
 *    Constructs a UDP datagram according to the program protocol. Since
 *  creating the datagram involves creating a list of all the files that
 *  are shareable, if the number of files is too large the UDP datagram
 *  will not be able to support every file. In that case, the file list
 *  is truncated and an error message is sent to 'stderr'.
 */
void construct_udp_data(struct main_var *vars, char* data, size_t* data_len);
