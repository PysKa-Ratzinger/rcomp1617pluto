#pragma once

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
 *    Frees all the allocated memory used by the file_storage structure passed
 *  as an argument.
 */
void freefsinfo(struct file_storage* storage);
