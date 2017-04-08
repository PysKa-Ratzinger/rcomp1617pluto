CC=gcc
CFLAGS=-Wall -Wextra
LDLIBS=-lpthread -lrt

main: main.o init.o fork_utils.o file_storage.o broadcast.o utils.o udp_receiver.o

%.o: %.c %.h

clean:
	rm -fv *.o *.gch main
