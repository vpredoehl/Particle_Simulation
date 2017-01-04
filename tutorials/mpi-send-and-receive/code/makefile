EXECS=send_recv #ping_pong ring
MPICC?=mpic++ 
CFLAGS=-std=c++11 -g 

all: ${EXECS}

send_recv: send_recv.o MPIntf.o 
	${MPICC} -o $@ $^

send_recv.o:	MPIntf.h send_recv.cpp
MPIntf.o:	MPIntf.h MPIntf.cpp

%.o : %.c
	$(MPICC) -c $(CFLAGS)  $<

#ping_pong: ping_pong.c
#	${MPICC} -o ping_pong ping_pong.c

#ring: ring.c
#	${MPICC} -o ring ring.c

clean:
	rm -f ${EXECS} *.o
