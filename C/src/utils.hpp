#pragma once
#include <stdlib.h>
#include <time.h>

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
int num_places(unsigned int n);

/**
 *    Asks the user for a valid folder name until the folder name
 *  he supplies is, indeed, valid.
 */
void read_folder(char* folder, size_t buffer_size);

/**
 *    Asks the user for a valid nickname and confirmation. The resulting
 *  nick is stored in the buffer passed as an argument
 */
void read_nick(char* nick, size_t buffer_size);

/**
 *    Checks if the string passed as an argument has a '\n' before the
 *  null terminator. If it has, the '\n' is replaced with a null terminator,
 *  making the string 1 byte shorter. The new size of the string is
 *  returned.
 */
size_t remove_newline(char* string);

/**
 *    Checks if the string passed as an argument has a '\n' before the
 *  null terminator. If it has, the '\n' is replaced with a null terminator,
 *  making the string 1 byte shorter. The new size of the string is
 *  returned.
 */
size_t remove_n_newline(char* string, size_t string_size);

/**
 *    Compares two timespec structures and returns 0 if they are equal, 1 if
 *  the second timespec is after the first one and -1 if the first timespec
 *  is after the second.
 */
int compare_timespec(struct timespec* time1, struct timespec* time2);

/**
 *    Substracts optime to basetime.
 *    If an error occurs, -1 is returned.
 */
int substract_timespec(struct timespec* basetime, struct timespec* optime);

/**
 *    Adds optime to bastime.
 *    If an error occurs, -1 is returned.
 */
int add_timespec(struct timespec* basetime, struct timespec* optime);

/**
 *    Initializes timeres variable to the value of one second divided by
 *  updates_per_sec.
 */
int init_timeres(struct timespec* timeres, int updates_per_sec);

/**
 *    Converts a struct timespec information to a double. The value of the
 *  double will be equivalent to the number of seconds represented by the
 *  timeres passed as an argument.
 */
double timeres_to_double(struct timespec* timeres);
