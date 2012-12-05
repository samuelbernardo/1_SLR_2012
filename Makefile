CC = gcc
CFLAGS = -g -pg -Wall -pedantic -fno-pie -std='c99' -fopenmp
#MPIFLAGS = -g -pg -Wall -pedantic -fno-pie -std='c99'
MPIFLAGS = -Wall -pedantic -std='c99'
MPIOMPFLAGS = -Wall -pedantic -std='c99' -fopenmp
NUMNODES = 1
NUMPROCS = 1
NUMLAB = 11

recompile: clean compile

compile: docs-serial docs-mpi docs-omp-mpi

kinst: docs-serial-kinst docs-omp-kinst ren

docs-serial: docs-serial.c

docs-omp: docs-omp.c

docs-mpi: docs-mpi.c
	mpicc $(MPIFLAGS) $^ -o $@

docs-omp-mpi: docs-omp-mpi.c
	mpicc $(MPIOMPFLAGS) $^ -o $@

docs-serial-kinst: docs-serial.c
	kinst-ompp $(CC) $(CFLAGS) $^ -o $@

docs-omp-kinst: docs-omp.c
	kinst-ompp $(CC) $(CFLAGS) $^ -o $@

ren:
	mv docs-serial-kinst docs-serial
	mv docs-omp-kinst docs-omp

ta: ts tp

ts: ts1 ts2 ts3

tp: tp1 tp2 tp3

ts1: compile
	./docs-serial sampleDocInstances/ex5-1d.in > ex5-1d.tst
	diff ex5-1d.tst sampleDocInstances/ex5-1d.out > ex5-1d.diff

ts2: compile
	./docs-serial sampleDocInstances/ex10-2d.in > ex10-2d.tst
	diff ex10-2d.tst sampleDocInstances/ex10-2d.out > ex10-2d.diff

ts3: compile
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

td0: compile
	mpirun --host localhost -np $(NUMPROCS) ./docs-mpi sampleDocInstances/ex5-1d.in > ex5-1d.tst
	diff sampleDocInstances/ex5-1d.in.out sampleDocInstances/ex5-1d.out > ex5-1d.diff
	mpirun --host localhost -np $(NUMPROCS) ./docs-mpi sampleDocInstances/ex10-2d.in > ex10-2d.tst
	diff sampleDocInstances/ex10-2d.in.out sampleDocInstances/ex10-2d.out > ex10-2d.diff
	mpirun --host localhost -np $(NUMPROCS) ./docs-mpi sampleDocInstances/ex1000-50d.in > ex1000-50d.tst
	diff sampleDocInstances/ex1000-50d.in.out sampleDocInstances/ex1000-50d.out > ex1000-50d.diff

td1: compile
	availableNodes $(NUMNODES) > nodeslist.txt
	sed -i 's/,/\n/g' nodeslist.txt
	mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-mpi sampleDocInstances/ex5-1d.in > ex5-1d.tst
	diff ex5-1d.tst sampleDocInstances/ex5-1d.out > ex5-1d.diff

td2: compile
	availableNodes $(NUMNODES) > nodeslist.txt
	sed -i 's/,/\n/g' nodeslist.txt
	mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-mpi sampleDocInstances/ex10-2d.in > ex10-2d.tst
	diff ex10-2d.tst sampleDocInstances/ex10-2d.out > ex10-2d.diff

td3: compile
	availableNodes $(NUMNODES) > nodeslist.txt
	sed -i 's/,/\n/g' nodeslist.txt
	mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-mpi sampleDocInstances/ex1000-50d.in > ex1000-50d.tst
	diff ex1000-50d.tst sampleDocInstances/ex1000-50d.out > ex1000-50d.diff

tpd0: compile
	mpirun --host localhost -np $(NUMPROCS) ./docs-omp-mpi sampleDocInstances/ex5-1d.in > ex5-1d.tst
	diff sampleDocInstances/ex5-1d.in.out sampleDocInstances/ex5-1d.out > ex5-1d.diff
	mpirun --host localhost -np $(NUMPROCS) ./docs-omp-mpi sampleDocInstances/ex10-2d.in > ex10-2d.tst
	diff sampleDocInstances/ex10-2d.in.out sampleDocInstances/ex10-2d.out > ex10-2d.diff
	mpirun --host localhost -np $(NUMPROCS) ./docs-omp-mpi sampleDocInstances/ex1000-50d.in > ex1000-50d.tst
	diff sampleDocInstances/ex1000-50d.in.out sampleDocInstances/ex1000-50d.out > ex1000-50d.diff

testbig: tbs tbp tbd

tbs: compile
	./docs-serial sampleDocInstances/ex10m-100d.in > ex10m-100d.tst
	diff ex10m-100d.tst sampleDocInstances/ex10m-100d.out > ex10m-100d.diff
#	./docs-serial sampleDocInstances/ex100m-100d.in > ex100m-100d.tst
#	diff ex100m-100d.tst sampleDocInstances/ex100m-100d.out > ex100m-100d.diff
#	./docs-serial sampleDocInstances/ex1M-100d.in > ex1M-100d.tst
#	diff ex1M-100d.tst sampleDocInstances/ex1M-100d.out > ex1M-100d.diff

tbp: compile
	./docs-omp sampleDocInstances/ex10m-100d.in > ex10m-100d.tst
	diff ex10m-100d.tst sampleDocInstances/ex10m-100d.out > ex10m-100d.diff
	./docs-omp sampleDocInstances/ex100m-100d.in > ex100m-100d.tst
	diff ex100m-100d.tst sampleDocInstances/ex100m-100d.out > ex100m-100d.diff
	./docs-omp sampleDocInstances/ex1M-100d.in > ex1M-100d.tst
	diff ex1M-100d.tst sampleDocInstances/ex1M-100d.out > ex1M-100d.diff

tbd: compile
	availableNodes $(NUMNODES) > nodeslist.txt
	sed -i 's/,/\n/g' nodeslist.txt
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-mpi sampleDocInstances/ex10m-100d.in > ex10m-100d.tst
	diff sampleDocInstances/ex10m-100d.in.out sampleDocInstances/ex10m-100d.out > ex10m-100d.diff
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-mpi sampleDocInstances/ex100m-100d.in > ex100m-100d.tst
	diff sampleDocInstances/ex100m-100d.in.out sampleDocInstances/ex100m-100d.out > ex100m-100d.diff
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-mpi sampleDocInstances/ex100k-200-4-mod.in > ex100k-200-4-mod.tst
	diff sampleDocInstances/ex100k-200-4-mod.in.out sampleDocInstances/ex100k-200-4-mod.out > ex100k-200-4-mod.diff
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-mpi sampleDocInstances/ex1M-100d.in > ex1M-100d.tst
	diff sampleDocInstances/ex1M-100d.in.out sampleDocInstances/ex1M-100d.out > ex1M-100d.diff

tbpd: compile
	availableNodes $(NUMNODES) > nodeslist.txt
	sed -i 's/,/\n/g' nodeslist.txt
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-omp-mpi sampleDocInstances/ex10m-100d.in > ex10m-100d.tst
	diff sampleDocInstances/ex10m-100d.in.out sampleDocInstances/ex10m-100d.out > ex10m-100d.diff
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-omp-mpi sampleDocInstances/ex100m-100d.in > ex100m-100d.tst
	diff sampleDocInstances/ex100m-100d.in.out sampleDocInstances/ex100m-100d.out > ex100m-100d.diff
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-omp-mpi sampleDocInstances/ex100k-200-4-mod.in > ex100k-200-4-mod.tst
	diff sampleDocInstances/ex100k-200-4-mod.in.out sampleDocInstances/ex100k-200-4-mod.out > ex100k-200-4-mod.diff
	time mpirun --hostfile nodeslist.txt -np $(NUMNODES) ./docs-omp-mpi sampleDocInstances/ex1M-100d.in > ex1M-100d.tst
	diff sampleDocInstances/ex1M-100d.in.out sampleDocInstances/ex1M-100d.out > ex1M-100d.diff

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
	rm -f *.o *.*~ docs-serial docs-omp docs-mpi docs-omp-mpi *.diff val.txt *.tst nodeslist.txt sampleDocInstances/*.in.out 
	
clean-all:
	rm -f *.o *.*~ docs-serial docs-omp docs-mpi docs-omp-mpi *.diff val.txt *.tst docs-omp.c.opari.inc docs-omp.mod.c docs-serial.c.opari.inc docs-serial.mod.c ex1000-50d.in.*.ompp.txt ex10-2d.in.*.ompp.txt ex5-1d.in.*.ompp.txt gmon.out opari.rc opari.tab.c ex1M-100d.in.*.ompp.txt nodeslist.txt sampleDocInstances/*.in.out 
