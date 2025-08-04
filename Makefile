CC=clang
FLAGS=-std=c99 -Wall -Werror
LIBS=$(shell pkg-config --libs --cflags raylib)

all: main

main: main.c
	mkdir -p build
	$(CC) main.c $(LIBS) $(FLAGS) -o build/main

run:
	make
	./build/main

clean:
	rm -rf build
