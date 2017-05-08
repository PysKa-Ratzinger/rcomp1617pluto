CC=gcc
CFLAGS=-W -Wall -Wextra -Werror -Wfatal-errors
LDLIBS=-lpthread -lrt
OBJECTS=main.o init.o fork_utils.o file_storage.o broadcast.o utils.o udp_receiver.o p2p_cli.o server.o client.o progress_bar.o

main: $(OBJECTS)

%.o: %.c %.h

run: main
	./main

clean:
	rm -fv *.o *.gch main
