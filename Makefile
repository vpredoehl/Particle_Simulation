#
# Crane - HCC
#
# Intel Compilers are loaded by default; for other compilers please check the module list
#
CC = icpc
MPCC = mpicxx
OPENMP = -openmp
CFLAGS = -g -std=c++11
LIBS =


TARGETS = serial pthreads openmp mpi autograder

all:	$(TARGETS)

serial: serial.o common.o logging.o
	$(CC) -o $@ $(LIBS) serial.o common.o logging.o
autograder: autograder.o common.o logging.o
	$(CC) -o $@ $(LIBS) autograder.o common.o logging.o
pthreads: pthreads.o common.o logging.o
	$(CC) -o $@ $(LIBS) -lpthread pthreads.o common.o logging.o
openmp: openmp.o common.o logging.o
	$(CC) -o $@ $(LIBS) $(OPENMP) openmp.o common.o logging.o
mpi: mpi.o common.o logging.o
	$(MPCC) -o $@ $(LIBS) $(MPILIBS) mpi.o common.o logging.o

logging.o:	logging.cpp logging.h common.h
	$(CC) -c $(CFLAGS) logging.cpp
autograder.o: autograder.cpp common.h
	$(CC) -c $(CFLAGS) autograder.cpp
openmp.o: openmp.cpp common.h
	$(CC) -c $(OPENMP) $(CFLAGS) openmp.cpp
serial.o: serial.cpp common.h logging.h
	$(CC) -c $(CFLAGS) serial.cpp 
pthreads.o: pthreads.cpp common.h
	$(CC) -c $(CFLAGS) pthreads.cpp
mpi.o: mpi.cpp common.h
	$(MPCC) -c $(CFLAGS) mpi.cpp
common.o: common.cpp common.h logging.h
	$(CC) -c $(CFLAGS) common.cpp

clean:
	rm -f *.o $(TARGETS) *.stdout *.txt
