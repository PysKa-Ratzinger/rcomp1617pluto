#pragma once
/*******************************************************************
 *
 * FILE_STORAGE.H -- File information storage library
 *
 * Author: Ricardo Catalao
 *
 * Purpose: Provide auxiliary methods for file information
 *            handling
 */

struct file_info;
struct file_storage;

#include "init.h"
#include "udp_receiver.h"

#define FILENAME_SIZE 256

 /**
  *    file_info structure
  *  Used to save important information regarding some files
  *
  *  Members:
  *    f_bytes   Number of bytes needed to represent the file name
  *    f_name    The name of the file
  *    f_next    Pointer to the next file_info structure or NULL if there
  *                isn't one
  */
 struct file_info{
   int               f_bytes;  // We need this of UDP broadcasting
   char*             f_name;
   struct file_info* f_next;
 };

/**
 *    file_storage structure
 *  Used to save the name of the folder where the files are and also to store
 *    the head of the linked list where file information will be stored
 *
 *  Members:
 *    f_folder    Name of the folder where the files are located
 *    f_headfile  Pointer the the head file_info structure or NULL if there
 *                  are no files in this folder
 */
struct file_storage{
  char*             f_folder;
  struct file_info* f_headfile; // First file of the linked list
};

/**
 *    UDP Datagrams linked list
 */
struct udp_datagrams{
	char*                 u_data;
  size_t                u_len;
	struct udp_datagrams* u_next;
};

/**
 *    Given the name of a folder path, a file_storage structure is created
 *  and returned.
 *    The structure will also already have initialized a linked list where
 *  information about every file in the folder will be in.
 *
 *  RETURN VALUE:
 *    On success, returns a pointer to a valid file_storage structure. On
 *  error, return NULL.
 */
struct file_storage* createfsinfo(char* folder_name);

/**
 *    For a given file_storage structure, an update involves deleting the
 *  file_info linked list completely and remaking it in. That's what happens
 *  to the file_storage structure passed as an argument to this function.
 *
 *  RETURN VALUE:
 *    On success, 0 will be returned. In case of error, -1 is returned
 *  instead.
 */
int updatefs(struct file_storage* storage);

/**
 *    Frees all the allocated memory used by the linked list structure inside
 *  the file_storage structure passed as an argument
 *
 *  RETURN VALUE:
 *    On success, 0 will be returned. In case of error, -1 is returned
 *  instead.
 */
int cleanfs(struct file_storage* storage);

/**
 *    Frees the linked list of file_info structures starting with the one
 *  passed as an argument.
 */
void freeflistinfo(struct file_info* flist);

/**
 *    Frees all the allocated memory used by the file_storage structure passed
 *  as an argument.
 */
void freefsinfo(struct file_storage* storage);

/**
 *    Prints every file in storage to stdout
 */
void printfsinfo(struct file_storage* storage);

/**
 *    Prints every file in the list started by the file_info structures
 *  passed as an argument.
 */
void printflistinfo(struct file_info* head_file);

/**
 *    Constructs a UDP datagram according to the program protocol. Since
 *  creating the datagram involves creating a list of all the files that
 *  are shareable, if the number of files is too large the UDP datagram
 *  will not be able to support every file. In that case, the file list
 *  is truncated and an error message is sent to 'stderr'.
 */
struct udp_datagrams* construct_udp_data(struct main_var *vars,
                                        struct file_storage* storage,
                                        unsigned short id);

/**
 *    Frees the memory allocated for the udp_datagrams
 */
void free_udp_datagrams(struct udp_datagrams* target);

/**
 *    Parses the information of a UDP datagram.
 */
int parse_datagram(char *buffer, size_t buffer_max_size,
                  struct peer_info *peer_info);
