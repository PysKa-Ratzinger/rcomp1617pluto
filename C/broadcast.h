#pragma once

#include "init.h"

#define UDP_DATAGRAM_MAX_SIZE 65508
#define SEM_BCAST_NAME "PLUTO_BCAST_SEM"

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
 *
 *  RETURNS:
 *    Returns the child pid of the created process.
 */
int start_broadcast(struct main_var *vars, char *folder);

/**
 *    Stops the udp packet broadcasting for the process corresponding
 *  to the pid passed as an argument
 */
void stop_broadcast(int pid);
