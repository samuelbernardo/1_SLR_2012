CC = gcc
CFLAGS = -g -Wall -pedantic


recompile: clean compile

compile: docs-serial

docs-serial: docs-serial.c

test: 
	./docs-serial sampleDocInstances/ex10-2d.in
	
val:
	valgrind ./docs-serial sampleDocInstances/ex10-2d.in

fullval:
	valgrind --leak-check=full --show-reachable=yes ./docs-serial sampleDocInstances/ex10-2d.in

all: recompile test

clean:
	rm -f *.o *.*~ docs-serial