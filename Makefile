all: main

main: main.c init.c init.h
	gcc -Wall -Wextra -o main main.c init.c

clean:
	rm -fv main

