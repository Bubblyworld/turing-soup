compile: main.c soup.c soup.h
		gcc -o soup main.c soup.c -I. -Wall -O3

run: compile
		./soup

debug: compile
		gdb ./soup

