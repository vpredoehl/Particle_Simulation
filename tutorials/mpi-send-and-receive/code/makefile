EXECS=send_recv #ping_pong ring
MPICC?=mpic++ 
CFLAGS=-std=c++11 -g 

all: ${EXECS}

send_recv: send_recv.o ParallelTask.o
	${MPICC} -o $@ $^

ParallelTask.cpp:	ParallelTask.h

send_recv.o:	send_recv.cpp MPIntf.cpp MPIntf.h 
	$(MPICC) -c $(CFLAGS)  $<
ParallelTask.o:	ParallelTask.cpp ParallelTask.h
	$(MPICC) -c $(CFLAGS)  $<

%.o : %.c
	$(MPICC) -c $(CFLAGS)  $<

#ping_pong: ping_pong.c
#	${MPICC} -o ping_pong ping_pong.c

#ring: ring.c
#	${MPICC} -o ring ring.c

clean:
	rm -f ${EXECS} *.o
