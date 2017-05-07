#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

#include "init.h"
#include "client.h"
#include "utils.h"

#define BUFFER_SIZE 4096

unsigned long start_file_transfer(const struct in_addr bf_addr,
                                  const unsigned short port,
                                  const char* file, const char* location){
    unsigned long res = 0;
    int sock_tcp, nbyte;
    unsigned int filestrlen, filestrlenlen;
    unsigned long file_size, remaining_nbytes;
    size_t totalsize, index;
    char *dyn_buffer;
    struct sockaddr_in addr;
    char buffer[512];
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

    dyn_buffer = malloc(totalsize+1);
    snprintf(dyn_buffer, totalsize+1, "%d:%s", filestrlen, file);

    // --------------- OPEN OUTPUT FILE -------------------------------

    printf("Opening output file...\n");

    output_file = fopen(location, "r+");
    if(output_file != NULL){    // File does not exist
        printf("Output file already exists. "
               "Do you want to overwrite it? [y/N]");
        fgets(buffer, BUFFER_SIZE, stdin);
        if(buffer[0] != 'y' && buffer[0] != 'Y'){
            close(sock_tcp);
            return -1;
        }
    }

    output_file = fopen(location, "w+");
    if(output_file == NULL){    // Can't create file
        perror("fopen");
        printf("Cannot create output file. Do you have the "
               "necessary folder permissions?");
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
        if(index > 11){
            printf("File size over 10GB. Canceling download.\n");
            close(sock_tcp);
            fclose(output_file);
            unlink(location);
            return -1;
        }
    }while(buffer[index-1] != ':');
    buffer[index-1] = '\0';

    // ---------------- RECEIVE FILE -------------------------------

    printf("Receiving file...\n");

    file_size = atol(buffer);
    remaining_nbytes = file_size;
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
    }

    // ------------------- SUCCESS -----------------

    printf("File download successful.\n");
    close(sock_tcp);
    fclose(output_file);
    return res;
}
