#pragma once

/*******************************************************************
 *
 * FORK_UTILS.H -- Process forking auxiliary library
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Provide auxiliary methods for handling methods that
 *            require the need for child processes to be created
 */

/**
 *   Creates 'num' child processes and returns 0 for the parent
 * process or 1,2,...,num for each of the childs created.
 *   If an error occurs in any given fork, -1 shall be returned
 * at the time the error occurs. Nothing can be done for the processes
 * that were created up to that point.
 */
int create_childs(int num);

/**
 *   Executes a command and returns the output for reading
 */
int psystem(char *cmd);
