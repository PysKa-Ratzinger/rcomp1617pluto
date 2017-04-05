CC=gcc
CFLAGS=-Wall -Wextra

main: main.o init.o fork_utils.o file_storage.o broadcast.o

init.o: init.c init.h 
fork_utils.o: fork_utils.c fork_utils.h 
file_storage.o: file_storage.c file_storage.h 
broadcast.o: broadcast.c broadcast.h
 
clean: 
	rm -fv *.o main  
