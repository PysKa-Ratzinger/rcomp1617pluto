#pragma once

#include "init.hpp"

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
int start_broadcast(struct main_var *vars, char *folder,
                    struct control_st *ctrl);

/**
 *    Stops the udp packet broadcasting for the process corresponding
 *  to the pid passed as an argument
 */
void stop_broadcast(int pid);
