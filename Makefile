CC=gcc
flags = -ggdb

scanning : scanning.c javastructures.h
	$(CC) scanning.c $(flags) -o scanning -DSCANNING_MAIN

parsing: scanning.c scanning.h parsing.c javastructures.h
	$(CC) scanning.c parsing.c $(flags) -o parsing -DPARSING_MAIN 
