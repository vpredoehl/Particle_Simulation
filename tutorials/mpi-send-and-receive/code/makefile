EXECS=send_recv #ping_pong ring
MPICC?=mpic++ 
CFLAGS=-std=c++11 -g 

all: ${EXECS}

send_recv: send_recv.o ParallelTask.o
	${MPICC} -o $@ $^

ParallelTask.cpp:	ParallelTask.h

send_recv.o:	MPIntf.h send_recv.cpp
ParallelTask.o:	ParallelTask.cpp

%.o : %.c
	$(MPICC) -c $(CFLAGS)  $<

#ping_pong: ping_pong.c
#	${MPICC} -o ping_pong ping_pong.c

#ring: ring.c
#	${MPICC} -o ring ring.c

clean:
	rm -f ${EXECS} *.o
