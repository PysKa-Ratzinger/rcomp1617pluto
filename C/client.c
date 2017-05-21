#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

#include "init.h"
#include "client.h"
#include "utils.h"
#include "progress_bar.h"

#define BUFFER_SIZE   4096
#define MAX_FILE_SIZE 19

unsigned long start_file_transfer(const struct in_addr bf_addr,
                                  const unsigned short port,
                                  const char* file, const char* location){
    unsigned long res = 0;
    int sock_tcp, nbyte;
    unsigned int filestrlen, filestrlenlen, temp;
    unsigned long long file_size, remaining_nbytes;
    size_t totalsize, index;
    char *dyn_buffer;
    struct sockaddr_in addr;
    char full_path_file[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    FILE* output_file;

    // ------------------- INITIALIZE INFORMATION ------------------

    printf("Initializing information for file transfer...\n");

    memset(&addr, 0, sizeof(struct sockaddr_in));
    memcpy(&addr.sin_addr, &bf_addr, sizeof(struct in_addr));
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    if(init_tcp(&sock_tcp) != 0){
        perror("init_tcp");
        printf("Could not start tcp socket for file transfer.\n");
        close(sock_tcp);
        return -1;
    }

    filestrlen = strlen(file);
    filestrlenlen = num_places(filestrlen);
    totalsize = filestrlenlen + 1 + filestrlen;

    dyn_buffer = (char*) malloc(totalsize+1);
    snprintf(dyn_buffer, totalsize+1, "%d:%s", filestrlen, file);

    // --------------- OPEN OUTPUT FILE -------------------------------

    printf("Opening output file...\n");

    strcpy(full_path_file, location);
    temp = strlen(full_path_file);
    full_path_file[temp] = '/';
    temp++;
    strcpy(&full_path_file[temp], file);
    printf("File: %s\n", full_path_file);

    output_file = fopen(full_path_file, "r");
    if(output_file != NULL){    // File already exists
        fclose(output_file);
        printf("Output file already exists. "
               "Do you want to overwrite it? [y/N]");
        fgets(buffer, BUFFER_SIZE, stdin);
        if(buffer[0] != 'y' && buffer[0] != 'Y'){
            close(sock_tcp);
            return -1;
        }
    }

    output_file = fopen(full_path_file, "w");
    if(output_file == NULL){    // Can't create file
        perror("fopen");
        printf("Cannot create/overwrite output file. Do you have the "
               "necessary folder permissions?\n");
        close(sock_tcp);
        return -1;
    }

    // --------------- CONNECT TO PEER -------------------------------

    if(connect(sock_tcp, (struct sockaddr*)&addr, sizeof(addr)) != 0){
        perror("connect");
        printf("Could not connect to peer.\n");
        close(sock_tcp);
        fclose(output_file);
        unlink(location);
        return -1;
    }

    // --------------- SEND FILE REQUEST ------------------------------

    printf("Sending file request...\n");

    index = 0;
    nbyte = 0;
    while(index < totalsize){
        printf("Calling send...\n");
        nbyte = send(sock_tcp, dyn_buffer+index, totalsize-index, 0);
        printf("Send called.\n");
        if(nbyte == -1){
            perror("send");
            printf("Could not send file request.\n");
            close(sock_tcp);
            fclose(output_file);
            unlink(location);
            return -1;
        }
        index += nbyte;
    }
    free(dyn_buffer);

    // --------------- RECEIVE FILE INFORMATION ---------------------

    printf("Receiving file information...\n");

    index = 0;
    nbyte = 0;
    do{
        nbyte = recv(sock_tcp, &buffer[index], 1, 0);
        index += nbyte;
        if(nbyte == -1){
          perror("recv");
          close(sock_tcp);
          fclose(output_file);
          unlink(location);
          return -1;
        }else if(index > MAX_FILE_SIZE){
            printf("File size too big. Canceling download.\n");
            close(sock_tcp);
            fclose(output_file);
            unlink(location);
            return -1;
        }
    }while(buffer[index-1] != ':');
    buffer[index-1] = '\0';
    file_size = atol(buffer);
    fprintf(stderr, "File size is %llu bytes.\n", file_size);

    // ---------------- RECEIVE FILE -------------------------------

    printf("Receiving file...\n");
    remaining_nbytes = file_size;
    create_progress_bar();
    update_progress_bar(file_size - remaining_nbytes, file_size);
    while(remaining_nbytes){
        nbyte = recv(sock_tcp, buffer, (BUFFER_SIZE > remaining_nbytes
                     ? remaining_nbytes : BUFFER_SIZE), 0);
        if(nbyte < 0){
            perror("recv");
            close(sock_tcp);
            fclose(output_file);
            unlink(location);
            return -1;
        }else if(nbyte == 0){
            if(remaining_nbytes != 0){
                printf("Server disconnected mid-way through download.\n");
                close(sock_tcp);
                fclose(output_file);
                unlink(location);
                return -1;
            }
        }
        if(fwrite(buffer, 1, nbyte, output_file) != (unsigned)nbyte){
            printf("Could not write information into file.\n");
            close(sock_tcp);
            fclose(output_file);
            unlink(location);
            return -1;
        }
        remaining_nbytes -= nbyte;
        update_progress_bar(file_size - remaining_nbytes, file_size);
    }

    // ------------------- SUCCESS -----------------

    printf("File download successful.\n");
    close(sock_tcp);
    fclose(output_file);
    return res;
}
