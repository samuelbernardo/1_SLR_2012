CC = gcc
CFLAGS = -g -pg -Wall -pedantic -fno-pie -std='c99' -fopenmp


recompile: clean compile

compile: docs-serial docs-omp

kinst: docs-serial-kinst docs-omp-kinst ren

docs-serial: docs-serial.c

docs-omp: docs-omp.c

docs-serial-kinst: docs-serial.c
	kinst-ompp $(CC) $(CFLAGS) $^ -o $@

docs-omp-kinst: docs-omp.c
	kinst-ompp $(CC) $(CFLAGS) $^ -o $@

ren:
	mv docs-serial-kinst docs-serial
	mv docs-omp-kinst docs-omp

ta: ts tp

ts: t1 t2 t3

tp: tp1 tp2 tp3

t1: compile
	./docs-serial sampleDocInstances/ex5-1d.in > ex5-1d.tst
	diff ex5-1d.tst sampleDocInstances/ex5-1d.out > ex5-1d.diff

t2: compile
	./docs-serial sampleDocInstances/ex10-2d.in > ex10-2d.tst
	diff ex10-2d.tst sampleDocInstances/ex10-2d.out > ex10-2d.diff

t3: compile
	./docs-serial sampleDocInstances/ex1000-50d.in > ex1000-50d.tst
	diff ex1000-50d.tst sampleDocInstances/ex1000-50d.out > ex1000-50d.diff

tp1: compile
	./docs-omp sampleDocInstances/ex5-1d.in > ex5-1d.tst
	diff ex5-1d.tst sampleDocInstances/ex5-1d.out > ex5-1d.diff

tp2: compile
	./docs-omp sampleDocInstances/ex10-2d.in > ex10-2d.tst
	diff ex10-2d.tst sampleDocInstances/ex10-2d.out > ex10-2d.diff

tp3: compile
	./docs-omp sampleDocInstances/ex1000-50d.in > ex1000-50d.tst
	diff ex1000-50d.tst sampleDocInstances/ex1000-50d.out > ex1000-50d.diff

testbig: compile
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
	rm -f *.o *.*~ docs-serial docs-omp *.diff val.txt *.tst
	
clean-all:
	rm -f *.o *.*~ docs-serial docs-omp *.diff val.txt *.tst docs-omp.c.opari.inc docs-omp.mod.c docs-serial.c.opari.inc docs-serial.mod.c ex1000-50d.in.*.ompp.txt ex10-2d.in.*.ompp.txt ex5-1d.in.*.ompp.txt gmon.out opari.rc opari.tab.c 
