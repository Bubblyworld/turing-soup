run: main.c soup.c soup.h
		gcc -o soup main.c soup.c -I. -Wall -O3
		./soup

debug: main.c soup.c soup.h
		gcc -o soup main.c soup.c -I. -Wall -g
		gdb ./soup
