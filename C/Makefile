all: main

main: main.o init.o fork_utils.o file_storage.o
main.o: main.c
init.o: init.c
fork_utils.o: fork_utils.c
file_storage.o: file_storage.c

clean:
	rm -fv *.o main

