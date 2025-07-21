CC=clang
FLAGS=-std=c99 -Wall
LIBS=$(shell pkg-config --libs --cflags raylib)

all: main

main: main.c
	$(CC) main.c $(LIBS) $(FLAGS) -o out

clean:
	rm ./out

