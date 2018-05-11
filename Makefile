CC=gcc
flags = -ggdb

parsingObj=parsing.o scanning.o types.o util.o
parsing: $(parsingObj) 
	$(CC) -c main.c -DPARSING_MAIN
	$(CC) $(parsingObj) main.o $(flags) -o parsing -DPARSING_MAIN 

scanningObj=scanning.o util.o
scanning : $(scanningObj) 
	$(CC) -c main.c -DSCANNING_MAIN
	$(CC) $(scanningObj) main.o  $(flags) -o scanning -DSCANNING_MAIN

parsing.o : types.h parsing.h parsing.c scanning.h util.h 
	$(CC) -c parsing.c

scanning.o : scanning.c javastructures.h scanning.h util.h
	$(CC) -c scanning.c

types.o : types.c types.h
	$(CC) -c types.c

utils.o : util.c util.h
	$(CC) -c util.c

types.c : types.h

types.h : types.enum
	python enum.py types.enum

.PHONY: clean
clean : 
	rm *.o types.c types.h
