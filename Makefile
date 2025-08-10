CC=clang
FLAGS=-std=c99 -Wall -Werror
DEV=-fcolor-diagnostics -fansi-escape-codes -fsanitize=address -g
RELEASE=-O3
LIBS=$(shell pkg-config --libs --cflags raylib)

all: main

main: main.c
	mkdir -p build
	$(CC) main.c $(LIBS) $(FLAGS) $(DEV) -o build/main

release:
	mkdir -p build
	$(CC) main.c $(LIBS) $(FLAGS) $(RELEASE) -o build/main

run:
	make main
	./build/main

clean:
	rm -rf build
