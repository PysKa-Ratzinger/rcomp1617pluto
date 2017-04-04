#include <unistd.h>

/**
 *   Creates 'num' child processes and returns 0 for the parent
 * process or 1,2,...,num for each of the childs created.
 *   If an error occurs in any given fork, -1 shall be returned
 * at the time the error occurs. Nothing can be done for the processes
 * that were created up to that point.
 */
int create_childs(int num);
