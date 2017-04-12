#pragma once

/*******************************************************************
 *
 * P2P_CLI.H -- Command line interface library
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Provide a simple usable command line interface to
 *            interact with the user.
 */

 #define SEM_OP_NAME "PLUTO_OP_SEM"

/**
 *    Starts the command line interface
 */
 void start_cli(struct control_st *ctrl);
