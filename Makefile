CC = gcc
CFLAGS = -g -pg -Wall -pedantic


recompile: clean compile

compile: docs-serial

docs-serial: docs-serial.c

test: 
#	./docs-serial sampleDocInstances/ex10-2d.in
	./docs-serial sampleDocInstances/ex5-1d.in > t.out
	diff t.out sampleDocInstances/ex5-1d.out

val:
#	valgrind ./docs-serial sampleDocInstances/ex10-2d.in
	valgrind ./docs-serial sampleDocInstances/ex5-1d.in

fullval:
#	valgrind --leak-check=full --show-reachable=yes ./docs-serial sampleDocInstances/ex10-2d.in
	valgrind --leak-check=full --show-reachable=yes ./docs-serial sampleDocInstances/ex5-1d.in

profile:
	

all: recompile test

clean:
	rm -f *.o *.*~ docs-serial *.out
