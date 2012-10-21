CC = gcc
CFLAGS = -g -pg -Wall -pedantic -fno-pie


recompile: clean compile

compile: docs-serial

docs-serial: docs-serial.c

test:
	./docs-serial sampleDocInstances/ex5-1d.in > ex5-1d.tst
	diff ex5-1d.tst sampleDocInstances/ex5-1d.out > ex5-1d.diff
	./docs-serial sampleDocInstances/ex10-2d.in > ex10-2d.tst
	diff ex10-2d.tst sampleDocInstances/ex10-2d.out > ex10-2d.diff
	./docs-serial sampleDocInstances/ex1000-50d.in > ex1000-50d.tst
	diff ex1000-50d.tst sampleDocInstances/ex1000-50d.out > ex1000-50d.diff

testbig:
	./docs-serial sampleDocInstances/ex1M-100d.in > ex1M-100d.tst
	diff ex1M-100d.tst sampleDocInstances/ex1M-100d.out > ex1M-100d.diff

val:
#    valgrind ./docs-serial sampleDocInstances/ex10-2d.in
#    valgrind ./docs-serial sampleDocInstances/ex5-1d.in
	valgrind ./docs-serial sampleDocInstances/ex1M-100d.in

fullval:
#    valgrind --leak-check=full --show-reachable=yes ./docs-serial sampleDocInstances/ex10-2d.in
	valgrind --leak-check=full --show-reachable=yes ./docs-serial sampleDocInstances/ex5-1d.in

supertest: test
	echo Testing ex5-1d: > val.txt
	valgrind ./docs-serial sampleDocInstances/ex5-1d.in >> val.txt
	echo End of ex5-1d... >> val.txt
	echo Testing ex10-2d: >> val.txt
	valgrind ./docs-serial sampleDocInstances/ex10-2d.in >> val.txt
	echo End of ex10-2d... >> val.txt
	echo Testing ex1000-50d: >> val.txt
	valgrind ./docs-serial sampleDocInstances/ex1000-50d.in >> val.txt
	echo End of ex1000-50d... >> val.txt

insanetest:
	echo Testing ex1M-100d: > val.txt
	valgrind ./docs-serial sampleDocInstances/ex1M-100d.in >> val.txt

profile:


all: recompile test

clean:
	rm -f *.o *.*~ docs-serial *.diff val.txt *.tst *.out 
