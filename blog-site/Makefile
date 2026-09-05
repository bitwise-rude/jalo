CC=gcc
CFLAGS=../jalo.c ../http.c
IFLAGS=-I../lua-5.5.1/src/
LFLAGS=-L../lua-5.5.1/src/

all : main

main: main.c 
	$(CC) $(IFLAGS) main.c $(CFLAGS) $(LFLAGS) -llua -lm -o main

clean: 
	rm -rf main



