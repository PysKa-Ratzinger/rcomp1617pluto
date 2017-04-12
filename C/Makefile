CC=gcc
CFLAGS=-Wall -Wextra
LDLIBS=-lpthread -lrt
OBJECTS=main.o init.o fork_utils.o file_storage.o broadcast.o utils.o udp_receiver.o p2p_cli.o

main: $(OBJECTS)

%.o: %.c %.h

clean:
	rm -fv *.o *.gch main
