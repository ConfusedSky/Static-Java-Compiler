CC=gcc
flags = -ggdb

ssaObj=ssa.o parsing.o scanning.o types.o util.o bytecodes.o main.c
ssa: $(ssaObj)
	$(CC) $(ssaObj) $(flags) -o ssa -DSSA_MAIN

interprettingObj=interpretting.o parsing.o scanning.o types.o util.o bytecodes.o main.c
interpret: $(interprettingObj)
	$(CC) $(interprettingObj) $(flags) -o interpret -DINTERPRETTING_MAIN

parsingObj=parsing.o scanning.o types.o util.o bytecodes.o main.c
parsing: $(parsingObj) 
	$(CC) $(parsingObj) $(flags) -o parsing -DPARSING_MAIN 

scanningObj=scanning.o util.o main.c
scanning : $(scanningObj) 
	$(CC) $(scanningObj) $(flags) -o scanning -DSCANNING_MAIN

ssa.o : types.h ssa.c ssa.c util.h bytecodes.h cstructures.h
	$(CC) -c ssa.c

interpretting.o : types.h interpretting.c interpretting.h util.h bytecodes.h cstructures.h scanning.h
	$(CC) -c interpretting.c

parsing.o : types.h parsing.h parsing.c scanning.h util.h bytecodes.h javastructures.h cstructures.h
	$(CC) -c parsing.c

scanning.o : scanning.c javastructures.h scanning.h util.h
	$(CC) -c scanning.c

types.o : types.c types.h
	$(CC) -c types.c

utils.o : util.c util.h
	$(CC) -c util.c

bytecodes.o : bytecodes.h bytecodes.c
	$(CC) -c bytecodes.c

types.c : types.h

types.h : types.enum
	python enum.py types.enum

.PHONY: clean
clean : 
	rm *.o types.c types.h
