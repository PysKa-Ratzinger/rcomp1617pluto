#pragma once
#include <stdlib.h>

/*******************************************************************
 *
 * UTILS.H -- Utilitary library
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Small functions that have nowhere else to go gather in
 *            here, in the hopes that humble people might need them
 *            in the short future.
 */

/**
 * Returns the number of digits needed to print a short number
 */
int num_places(unsigned short n);

/**
 *    Asks the user for a valid folder name until the folder name
 *  he supplies is, indeed, valid.
 */
void read_folder(char* folder, size_t buffer_size);
