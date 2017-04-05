#pragma once

#include "init.h"

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
void start_broadcast(struct main_var *vars);
