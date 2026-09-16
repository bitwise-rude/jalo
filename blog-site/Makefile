all: main

main: main.c
	gcc -I../  -I../lua-5.5.1/src/ main.c ../jalo.c  ../http.c -L../lua-5.5.1/src/ -llua -lm -o main

clean: 
	rm main -f
