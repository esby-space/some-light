CC=clang
FLAGS=-std=c99 -Wall -Werror
LIBS=$(shell pkg-config --libs --cflags raylib)

all: main

main: main.c
	$(CC) main.c $(LIBS) $(FLAGS) -o build/main

clean:
	rm ./main

